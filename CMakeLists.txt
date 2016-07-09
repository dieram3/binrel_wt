cmake_minimum_required(VERSION 3.5.2)
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

find_package(Doxygen)

# ---------------------------------------
# Project options
# ---------------------------------------

option(ENABLE_CLANG_MODULES "Enables support for clang modules" OFF)
option(COMPILE_BENCHMARKS "Compiles the library benchmarks" ON)

# ---------------------------------------
# Handle options
# ---------------------------------------

if(ENABLE_CLANG_MODULES)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-fmodules -fcxx-modules)
endif()
endif()

# ---------------------------------------
# Add subdirectories
# ---------------------------------------

add_subdirectory("lib")
add_subdirectory("docs")

# Add the unit tests
enable_testing()
add_subdirectory("test")
include(CTest)

if(COMPILE_BENCHMARKS)
  add_subdirectory("benchmarks")
endif()
