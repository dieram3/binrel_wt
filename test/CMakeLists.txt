find_package(doctest REQUIRED)

function(add_unittest TEST_NAME)
  set(TARGET_ID "${TEST_NAME}_test")
  add_brwt_executable(${TARGET_ID} ${ARGN})
  target_link_libraries(${TARGET_ID} PRIVATE brwt doctest::doctest)
  add_test(NAME ${TEST_NAME} COMMAND ${TARGET_ID})

  if(NOT BRWT_BUILD_TESTS)
    set_target_properties(${TARGET_ID} PROPERTIES EXCLUDE_FROM_ALL ON)
  endif()
endfunction()

add_subdirectory("brwt")
