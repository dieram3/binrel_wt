add_brwt_library(
  brwt
  "binary_relation.cpp"
  "implementation/bit_vector.cpp"
  "implementation/bitmap.cpp"
  "implementation/int_vector.cpp"
  "wavelet_tree/algorithms.cpp"
  "wavelet_tree/wavelet_tree.cpp"
)

target_sources(
  brwt
  PUBLIC
    FILE_SET CXX_MODULES
    FILES
      "interface/bit_ops.cpp"
      "interface/bit_vector.cpp"
      "interface/bitmap.cpp"
      "interface/common_types.cpp"
      "interface/concepts.cpp"
      "interface/index_range.cpp"
      "interface/int_vector.cpp"
      "interface/type_traits.cpp"
      "interface/utility.cpp"

      # FIXME: These could be private if they were no checked in unit tests.
      "detail/interface/iterator.cpp"
      "detail/interface/utility.cpp"
  # PRIVATE
  #   FILE_SET detail
  #   TYPE CXX_MODULES
  #   FILES
  #     "detail/iterator.cppm"
  #     "detail/utility.cppm"
)

target_include_directories(brwt PUBLIC
  "${CMAKE_SOURCE_DIR}/include"
)

install(TARGETS brwt DESTINATION lib)
