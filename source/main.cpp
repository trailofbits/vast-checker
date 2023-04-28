/*
 * Copyright (c) 2023-present, Trail of Bits, Inc.
 * All rights reserved.
 *
 * This source code is licensed in accordance with the terms specified in
 * the LICENSE file found in the root directory of this source tree.
 */

#include <vast/Util/Warnings.hpp>

VAST_RELAX_WARNINGS
#include <mlir/InitAllDialects.h>
#include <mlir/Target/LLVMIR/Dialect/All.h>
#include <mlir/Tools/mlir-opt/MlirOptMain.h>
VAST_UNRELAX_WARNINGS

#include <vast/Dialect/Dialects.hpp>

#include "sequoia_checker.hpp"

auto main(int argc, char** argv) -> int
{
  // register dialects
  mlir::DialectRegistry registry;
  vast::registerAllDialects(registry);
  mlir::registerAllDialects(registry);

  register_sequoia_checker_pass();

  return mlir::failed(
      mlir::MlirOptMain(argc, argv, "VAST Sequoia Bug Checker\n", registry));
}
