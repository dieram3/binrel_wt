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

# find_package(Doxygen)

# ---------------------------------------
# Add subdirectories
# ---------------------------------------

#add_subdirectory("src")   # Add the library

# Add the unit tests
# enable_testing()
# add_subdirectory("test")
# include(CTest)
