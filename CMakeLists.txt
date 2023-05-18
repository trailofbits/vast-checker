#
# Copyright (c) 2023, Trail of Bits, Inc. All rights reserved.
#
# This source code is licensed in accordance with the terms specified in the
# LICENSE file found in the root directory of this source tree.
#

cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
  vast-checker
  VERSION 0.1.0
  DESCRIPTION "Short description"
  HOMEPAGE_URL "https://github.com/trailofbits/vast-checker.git"
  LANGUAGES CXX)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Add external projects ----

add_subdirectory(external)

# ---- Add LLVM ----

find_package(LLVM 16 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# ---- Add MLIR ----

find_package(MLIR REQUIRED CONFIG)
message(STATUS "Using MLIRConfig.cmake in: ${MLIR_DIR}")

# ---- Declare library ----

add_library(vast-checker_lib OBJECT source/sequoia_checker.cpp)

target_include_directories(
  vast-checker_lib ${warning_guard}
  PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
         "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/external/vast/include>"
         "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/external/vast/include>"
         "$<BUILD_INTERFACE:${LLVM_INCLUDE_DIRS}>")

target_compile_features(vast-checker_lib PUBLIC cxx_std_20)

# ---- Declare executable ----

add_executable(vast-checker_exe source/main.cpp)
add_executable(vast-checker::exe ALIAS vast-checker_exe)

set_property(TARGET vast-checker_exe PROPERTY OUTPUT_NAME vast-checker)

target_compile_features(vast-checker_exe PRIVATE cxx_std_20)

get_property(DIALECT_LIBS GLOBAL PROPERTY MLIR_DIALECT_LIBS)
get_property(CONVERSION_LIBS GLOBAL PROPERTY MLIR_CONVERSION_LIBS)

target_link_libraries(
  vast-checker_exe
  PRIVATE vast-checker_lib
          ${DIALECT_LIBS}
          ${CONVERSION_LIBS}
          MLIROptLib
          MLIRHighLevel
          MLIRHighLevelTransforms
          MLIRCore
          MLIRMeta
          VASTConversions
          VASTCoreConversionPasses
          VASTFromHL
          VASTCommonConversionPasses)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT vast-checker_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
    AUTHOR_WARNING "Developer mode is intended for developers of vast-checker")
endif()

include(cmake/dev-mode.cmake)
