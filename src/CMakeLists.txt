cpl_add_library(binrel_wt
  "bit_vector.cpp"
  #"bitmap.cpp"
  "int_vector.cpp"
)

target_include_directories(binrel_wt PUBLIC
  "${CMAKE_SOURCE_DIR}/include"
)
