/*
 * Copyright (c) 2023-present, Trail of Bits, Inc.
 * All rights reserved.
 *
 * This source code is licensed in accordance with the terms specified in
 * the LICENSE file found in the root directory of this source tree.
 */

#pragma once

#include <mlir/Pass/Pass.h>
#include <vast/Dialect/HighLevel/HighLevelOps.hpp>
#include <vast/Dialect/HighLevel/HighLevelTypes.hpp>

#include "mlir/IR/Operation.h"

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

    auto check_cast = [&](auto cast) -> bool
    {
      if (cast.getKind() == CastKind::IntegralCast) {
        auto from_ty = cast.getValue().getType();
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
   * @brief Walks the use-def chain of `opr` and checks if any value on the
   * chain is involved in pointer arithmetic.
   */
  static auto has_ptr_arith_use(mlir::Operation* opr) -> bool
  {
    if (opr == nullptr) {
      return false;
    }

    if (is_arith_op(opr) && has_ptr_operand(opr)) {
      return true;
    }

    return llvm::any_of(opr->getUsers(), has_ptr_arith_use);
  }

  /**
   * @brief Get callee `vast::hl::FuncOp` from `vast::hl::CallOp`.
   */
  static auto get_callee(vast::hl::CallOp call, vast::vast_module mod)
      -> vast::hl::FuncOp
  {
    using vast::hl::FuncOp;

    auto callee = call.getCallableForCallee();
    if (auto sym = callee.dyn_cast<mlir::SymbolRefAttr>()) {
      return mlir::dyn_cast_or_null<FuncOp>(
          mlir::SymbolTable::lookupSymbolIn(mod, sym));
    }

    return {};
  }

  /**
   * @brief Pass entry point.
   */
  void runOnOperation() override
  {
    using vast::vast_module;

    using vast::hl::CallOp;

    auto fop = getOperation();

    auto check_for_sequoia = [&](CallOp call)
    {
      for (const auto& arg : llvm::enumerate(call.getArgOperands())) {
        if (is_unsigned_to_signed_cast(arg.value().getDefiningOp())) {
          auto mod    = mlir::cast<vast_module>(getOperation()->getParentOp());
          auto callee = get_callee(call, mod);
          auto param  = callee.getArgument(arg.index());
          if (llvm::any_of(param.getUsers(), has_ptr_arith_use)) {
            llvm::errs()
                << "Call to `" << callee.getSymName() << "` in `"
                << fop.getSymName()
                << "` passes an unsigned value to a signed argument (index `"
                << arg.index()
                << "`) and then uses it in pointer arithmetic.\n";
          }
        }
      }
    };

    fop.walk(check_for_sequoia);
  }
};

void register_sequoia_checker_pass();
