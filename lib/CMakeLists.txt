cpl_add_library(binrel_wt
  "binary_relation.cpp"
  "bit_vector.cpp"
  "bitmap.cpp"
  "int_vector.cpp"
  "wavelet_tree.cpp"
)

target_include_directories(binrel_wt PUBLIC
  "${CMAKE_SOURCE_DIR}/include"
)
