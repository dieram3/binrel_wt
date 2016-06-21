function(add_unittest TEST_NAME)
  set(TARGET_ID "${TEST_NAME}_test")
  cpl_add_executable(${TARGET_ID} ${ARGN})
  target_link_libraries(${TARGET_ID} PRIVATE binrel_wt)
  target_include_directories(${TARGET_ID} SYSTEM PRIVATE
    "${CMAKE_SOURCE_DIR}/test/doctest/doctest")
  add_test(NAME ${TEST_NAME} COMMAND ${TARGET_ID})
endfunction()

add_subdirectory("brwt")
