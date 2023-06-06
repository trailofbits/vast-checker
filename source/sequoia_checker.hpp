/*
 * Copyright (c) 2023-present, Trail of Bits, Inc.
 * All rights reserved.
 *
 * This source code is licensed in accordance with the terms specified in
 * the LICENSE file found in the root directory of this source tree.
 */

#pragma once

#include <vector>

#include <mlir/Pass/Pass.h>
#include <vast/Dialect/HighLevel/HighLevelOps.hpp>
#include <vast/Dialect/HighLevel/HighLevelTypes.hpp>

/**
 * @brief Pass that checks for the sequoia bug in VAST `hl` dialect code.
 */
struct sequoia_checker_pass
    : public mlir::PassWrapper<sequoia_checker_pass,
                               mlir::OperationPass<vast::hl::FuncOp>>
{
  /**
   * @brief CLI argument.
   */
  auto getArgument() const -> llvm::StringRef final { return "sequoia"; }

  /**
   * @brief CLI Description.
   */
  auto getDescription() const -> llvm::StringRef final
  {
    return "Checks for the sequoia bug in VAST `hl` dialect code";
  }

  /**
   * @brief Checks if `opr` is an unsigned-to-signed integer cast.
   */
  auto is_unsigned_to_signed_cast(mlir::Operation* opr) -> bool
  {
    using vast::vast_module;

    using vast::hl::CastKind;
    using vast::hl::CStyleCastOp;
    using vast::hl::ImplicitCastOp;

    using vast::hl::TypedefType;

    using vast::hl::getBottomTypedefType;
    using vast::hl::isSigned;
    using vast::hl::isUnsigned;
    using vast::hl::strip_elaborated;

    auto check_cast = [&](auto cast) -> bool
    {
      if (cast.getKind() == CastKind::IntegralCast) {
        auto from_ty = strip_elaborated(cast.getValue().getType());
        if (auto typedef_ty = from_ty.template dyn_cast<TypedefType>()) {
          auto mod = mlir::cast<vast_module>(getOperation()->getParentOp());
          from_ty  = getBottomTypedefType(typedef_ty, mod);
        }
        return isUnsigned(from_ty) && isSigned(cast.getType());
      }
      return false;
    };

    return llvm::TypeSwitch<mlir::Operation*, bool>(opr)
        .Case<ImplicitCastOp, CStyleCastOp>(check_cast)
        .Default(/*defaultResult=*/false);
  }

  /**
   * @brief Checks if `opr` has a pointer type operand.
   */
  static auto has_ptr_operand(mlir::Operation* opr) -> bool
  {
    using vast::hl::PointerType;

    auto is_ptr_type = [](mlir::Value val) -> bool
    { return val.getType().isa<PointerType>(); };

    return llvm::any_of(opr->getOperands(), is_ptr_type);
  }

  /**
   * @brief Checks if `opr` is an arithmetic operation. List of operations
   * checked against is not exhaustive.
   */
  static auto is_arith_op(mlir::Operation* opr) -> bool
  {
    using vast::hl::AddIOp;
    using vast::hl::SubIOp;

    return llvm::TypeSwitch<mlir::Operation*, bool>(opr)
        .Case<AddIOp, SubIOp>([](mlir::Operation*) { return true; })
        .Default(/*defaultResult=*/false);
  }

  /**
   * @brief Walks the use-def chain of `val` and returns first value on the
   * chain is involved in pointer arithmetic.
   */
  static auto get_ptr_arith_use(mlir::Value val) -> mlir::Operation*
  {
    using op_vec = llvm::SmallVector<mlir::Operation*, 32>;

    op_vec work_list(val.user_begin(), val.user_end());

    while (!work_list.empty()) {
      auto* opr = work_list.front();
      work_list.erase(work_list.begin());

      if (opr == nullptr) {
        continue;
      }

      if (is_arith_op(opr) && has_ptr_operand(opr)) {
        return opr;
      }

      work_list.append(opr->user_begin(), opr->user_end());
    }

    return nullptr;
  }

  /**
   * @brief Checks if the value of `val` is guarded to be less than
   * something by walking regions from `val` upwards.
   */
  static auto is_guarded_lesser(mlir::Value val) -> bool { return false; }

  /**
   * @brief Pass entry point.
   */
  void runOnOperation() override
  {
    using vast::vast_module;

    using vast::hl::CallOp;

    using vast::hl::getCallee;

    auto fop = getOperation();

    auto check_for_sequoia = [&](CallOp call)
    {
      for (const auto& [idx, val] : llvm::enumerate(call.getArgOperands())) {
        if (is_unsigned_to_signed_cast(val.getDefiningOp())) {
          auto callee = getCallee(call);
          auto param  = callee.getArgument(idx);
          if (auto* pau = get_ptr_arith_use(param)) {
            llvm::errs()
                << "Call to `" << callee.getSymName() << "` in `"
                << fop.getSymName()
                << "` passes an unsigned value to a signed argument (index `"
                << idx << "`) and then uses it in pointer arithmetic.\n";
          }
        }
      }
    };

    fop.walk(check_for_sequoia);
  }
};

void register_sequoia_checker_pass();
