/*
 * Copyright (c) 2023-present, Trail of Bits, Inc.
 * All rights reserved.
 *
 * This source code is licensed in accordance with the terms specified in
 * the LICENSE file found in the root directory of this source tree.
 */

#include <vast/Util/Warnings.hpp>

VAST_RELAX_WARNINGS
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/ToolOutputFile.h>
#include <mlir/IR/Dialect.h>
#include <mlir/IR/MLIRContext.h>
#include <mlir/InitAllDialects.h>
#include <mlir/InitAllPasses.h>
#include <mlir/Pass/Pass.h>
#include <mlir/Pass/PassManager.h>
#include <mlir/Support/FileUtilities.h>
#include <mlir/Target/LLVMIR/Dialect/All.h>
#include <mlir/Tools/mlir-opt/MlirOptMain.h>
VAST_UNRELAX_WARNINGS

#include <vast/Conversion/Passes.hpp>
#include <vast/Dialect/Dialects.hpp>
#include <vast/Dialect/HighLevel/Passes.hpp>

auto main(int argc, char** argv) -> int
{
  mlir::registerAllPasses();
  // Register VAST passes here
  vast::hl::registerPasses();
  vast::registerConversionPasses();

  mlir::DialectRegistry registry;
  // register dialects
  vast::registerAllDialects(registry);
  mlir::registerAllDialects(registry);

  // register conversions
  mlir::registerAllToLLVMIRTranslations(registry);
  vast::hl::registerHLToLLVMIR(registry);

  return mlir::failed(
      mlir::MlirOptMain(argc, argv, "VAST Sequoia Bug Checker\n", registry));
}
