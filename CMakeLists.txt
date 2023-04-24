#
# Copyright (c) 2023, Trail of Bits, Inc.
# All rights reserved.
#
# This source code is licensed in accordance with the terms specified in
# the LICENSE file found in the root directory of this source tree.
#

cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    vast-checker
    VERSION 0.1.0
    DESCRIPTION "Short description"
    HOMEPAGE_URL "https://github.com/trailofbits/vast-checker.git"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Add external project ----

add_subdirectory(external)

# ---- Declare library ----

add_library(
    vast-checker_lib OBJECT
    source/lib.cpp
)

target_include_directories(
    vast-checker_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(vast-checker_lib PUBLIC cxx_std_20)

# ---- Declare executable ----

add_executable(vast-checker_exe source/main.cpp)
add_executable(vast-checker::exe ALIAS vast-checker_exe)

set_property(TARGET vast-checker_exe PROPERTY OUTPUT_NAME vast-checker)

target_compile_features(vast-checker_exe PRIVATE cxx_std_20)

target_link_libraries(vast-checker_exe PRIVATE vast-checker_lib)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT vast-checker_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of vast-checker"
  )
endif()

include(cmake/dev-mode.cmake)
