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

/**
 * @brief Pass that checks for the sequoia bug in VAST `hl` dialect code
 */
struct sequoia_checker_pass
    : public mlir::PassWrapper<sequoia_checker_pass,
                               mlir::OperationPass<vast::hl::FuncOp>>
{
  /**
   * @brief CLI argument
   */
  auto getArgument() const -> llvm::StringRef final { return "sequoia"; }

  /**
   * @brief CLI Description
   */
  auto getDescription() const -> llvm::StringRef final
  {
    return "Checks for the sequoia bug in VAST `hl` dialect code";
  }

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
          from_ty = getBottomTypedefType(typedef_ty, mod);
        }
        return isUnsigned(from_ty) && isSigned(cast.getType());
      }
      return false;
    };

    return llvm::TypeSwitch<mlir::Operation*, bool>(opr)
        .Case<ImplicitCastOp, CStyleCastOp>(check_cast)
        .Default([](mlir::Operation*) { return false; });
  }

  static auto has_ptr_arith_use(mlir::Operation* opr) -> bool
  {
    using vast::hl::AddIOp;
    using vast::hl::PointerType;

    if (opr == nullptr) {
      return false;
    }

    if (auto add = mlir::dyn_cast<AddIOp>(opr)) {
      if (add.getLhs().getType().isa<PointerType>()
          || add.getRhs().getType().isa<PointerType>())
      {
        return true;
      }
    }

    return llvm::any_of(opr->getUsers(), has_ptr_arith_use);
  }

  static auto get_function(mlir::CallInterfaceCallable callee,
                           vast::vast_module mod) -> vast::hl::FuncOp
  {
    using vast::hl::FuncOp;

    if (auto sym = callee.dyn_cast<mlir::SymbolRefAttr>()) {
      return mlir::dyn_cast_or_null<FuncOp>(
          mlir::SymbolTable::lookupSymbolIn(mod, sym));
    }

    return {};
  }

  /**
   * @brief Pass entry point
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
          auto mod = mlir::cast<vast_module>(getOperation()->getParentOp());
          auto callee = get_function(call.getCallableForCallee(), mod);
          auto param = callee.getArgument(arg.index());
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
