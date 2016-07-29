cmake_minimum_required(VERSION 3.6.1)
project("binrel_wt" CXX)

# ---------------------------------------
# Initial configuration
# ---------------------------------------

if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, defaulted to Release")
  set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# For external tools:
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(CPL_functions)

# ---------------------------------------
# Project options
# ---------------------------------------

option(BRWT_INCLUDE_TESTS "Generate build targets for the BRWT unit tests." ON)
option(BRWT_BUILD_TESTS "Include unit test targets in the default build target." OFF)

option(BRWT_INCLUDE_BENCHMARKS "Generate build targets for the BRWT benchmark tests." OFF)

option(BRWT_INCLUDE_DOCS "Generate build targets for the BRWT documentation." ON)

option(BRWT_ENABLE_MODULES "Enable support for clang modules." OFF)

# ---------------------------------------
# Handle compilation options
# ---------------------------------------

if(BRWT_ENABLE_MODULES)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-fmodules -fcxx-modules)
endif()
endif()

# ---------------------------------------
# Add subdirectories
# ---------------------------------------

add_subdirectory("lib")

# Add the unit tests
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
