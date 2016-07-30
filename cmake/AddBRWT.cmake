function(brwt_enable_all_warnings TARGET_ID)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${TARGET_ID} PRIVATE
      -Weverything
      -Wno-c++98-compat
      -Wno-c++98-compat-pedantic
      -Wno-padded
    )
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${TARGET_ID} PRIVATE
      -Wall
      -Wextra
    )
  endif()
endfunction()

function(brwt_make_cpp14_strict TARGET_ID)
  set_property(TARGET ${TARGET_ID} PROPERTY CXX_EXTENSIONS       OFF)
  set_property(TARGET ${TARGET_ID} PROPERTY CXX_STANDARD          14)
  set_property(TARGET ${TARGET_ID} PROPERTY CXX_STANDARD_REQUIRED ON)
endfunction()

# Default configuration for BRWT targets.
function(brwt_configure TARGET_ID)
  brwt_enable_all_warnings(${TARGET_ID})
  brwt_make_cpp14_strict(${TARGET_ID})
endfunction()

function(add_brwt_library TARGET_ID)
  add_library(${TARGET_ID} ${ARGN})
  brwt_configure(${TARGET_ID})
endfunction()

function(add_brwt_executable TARGET_ID)
  add_executable(${TARGET_ID} ${ARGN})
  brwt_configure(${TARGET_ID})
endfunction()
