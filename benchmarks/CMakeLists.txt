# To install Benchmark in OS X do:
# brew install google-benchmark

find_package(Benchmark)

if(NOT Benchmark_FOUND)
  message(STATUS "Benchmark library not found. Benchmarks will not be compiled")
  return()
endif()

function(add_benchmark BM_NAME)
  set(TARGET_ID "bm_${BM_NAME}")
  cpl_add_executable(${TARGET_ID} ${ARGN})
  target_link_libraries(${TARGET_ID} binrel_wt Benchmark::benchmark)

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${TARGET_ID} PRIVATE -Wno-global-constructors)
  endif()
endfunction()

add_subdirectory("brwt")
