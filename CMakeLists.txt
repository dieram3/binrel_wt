cmake_minimum_required(VERSION 3.21.3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, defaulted to Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the build type." FORCE)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For external tools

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

# ---------------------------------------
# Project
# ---------------------------------------

option(BRWT_INCLUDE_TESTS "Generate build targets for the BRWT unit tests." ON)
option(BRWT_BUILD_TESTS "Include unit test targets in the default build target." OFF)

option(BRWT_INCLUDE_BENCHMARKS "Generate build targets for the BRWT benchmark tests." OFF)
option(BRWT_BUILD_BENCHMARKS "Include benchmark targets in the default build target." OFF)

option(BRWT_INCLUDE_DOCS "Generate build targets for the BRWT documentation." ON)

option(BRWT_ENABLE_MODULES "Enable support for clang modules." OFF)

option(BRWT_ENABLE_LTO "Enable link time optimization." OFF)

# Autoselect vcpkg features based on selected options. If Vcpkg is not in use,
# VCPKG_MANIFEST_FEATURES is just ignored.
if(BRWT_INCLUDE_TESTS)
  list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()
if(BRWT_INCLUDE_BENCHMARKS)
  list(APPEND VCPKG_MANIFEST_FEATURES "benchmarks")
endif()

project("binrel_wt" CXX)

# ---------------------------------------
# Handle compilation options
# ---------------------------------------

if(BRWT_ENABLE_MODULES)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-fmodules -fcxx-modules)
  add_compile_options(-Wno-non-modular-include-in-module) # for <cassert>
endif()
endif()

include(HandleOptions)
include(AddBRWT)

# ---------------------------------------
# Add subdirectories
# ---------------------------------------

add_subdirectory("lib")

if(BRWT_INCLUDE_TESTS)
  enable_testing()
  add_subdirectory("test")
  include(CTest)
endif()

if(BRWT_INCLUDE_BENCHMARKS)
  add_subdirectory("benchmarks")
endif()

if(BRWT_INCLUDE_DOCS)
  add_subdirectory("docs")
endif()

# ---------------------------------------
# Install include headers
# ---------------------------------------

install(DIRECTORY "include/brwt"
        DESTINATION "include"
        FILES_MATCHING PATTERN "*.h")
