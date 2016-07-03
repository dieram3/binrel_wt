function(cpl_enable_all_warnings TARGET_ID)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${TARGET_ID} PRIVATE
      -Weverything
      -Wno-c++98-compat
      -Wno-c++98-compat-pedantic
      -Wno-padded
      -Wno-non-modular-include-in-module # for <cassert>
    )
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${TARGET_ID} PRIVATE
      -Wall
      -Wextra
    )
  endif()
endfunction()

function(cpl_make_cpp14_strict TARGET_ID)
  set_property(TARGET ${TARGET_ID} PROPERTY CXX_EXTENSIONS       OFF)
  set_property(TARGET ${TARGET_ID} PROPERTY CXX_STANDARD          14)
  set_property(TARGET ${TARGET_ID} PROPERTY CXX_STANDARD_REQUIRED ON)
endfunction()

# Default configuration for CPL targets.
function(cpl_configure TARGET_ID)
  cpl_enable_all_warnings(${TARGET_ID})
  cpl_make_cpp14_strict(${TARGET_ID})
endfunction()

function(cpl_add_library TARGET_ID)
  add_library(${TARGET_ID} ${ARGN})
  cpl_configure(${TARGET_ID})
endfunction()

function(cpl_add_executable TARGET_ID)
  add_executable(${TARGET_ID} ${ARGN})
  cpl_configure(${TARGET_ID})
endfunction()
