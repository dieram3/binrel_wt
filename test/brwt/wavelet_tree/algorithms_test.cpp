#include "doctest.h"
#include <brwt/wavelet_tree.h>

#include <array> // array

using brwt::wavelet_tree;
using brwt::symbol_id;
using brwt::int_vector;
using brwt::index_type;
using brwt::index_npos;

static constexpr symbol_id operator"" _sym(const unsigned long long value) {
  return static_cast<symbol_id>(value);
}

static constexpr auto to_signed(const size_t value) {
  return static_cast<ptrdiff_t>(value);
}

TEST_SUITE("wavelet_tree/algorithms");

TEST_CASE("[select_first] small alphabet") {
  std::array<unsigned, 24> arr = {
      {0, 2, 2, 1, 2, 3, 1, 3, 2, 1, 3, 0, 0, 1, 2, 0, 1, 0, 0, 0, 3, 3, 2, 1}};

  int_vector vec(arr.size(), /*bpe=*/2);
  for (size_t i = 0; i < arr.size(); ++i) {
    vec[to_signed(i)] = arr[i];
  }

  const auto wt = wavelet_tree(vec);

  REQUIRE(wt.size() == 24);

  const auto select_first = [&](index_type start, symbol_id min,
                                symbol_id max) {
    return brwt::select_first(wt, start, brwt::between<symbol_id>{min, max});
  };

  // seq = 02212 3132 1300 1201 0003 321

  CHECK(select_first(/*start=*/0, 0_sym, 1_sym) == 0);
  CHECK(select_first(/*start=*/0, 1_sym, 1_sym) == 3);

  CHECK(select_first(/*start=*/0, 0_sym, 2_sym) == 0);
  CHECK(select_first(/*start=*/0, 1_sym, 2_sym) == 1);
  CHECK(select_first(/*start=*/0, 2_sym, 2_sym) == 1);

  CHECK(select_first(/*start=*/0, 0_sym, 3_sym) == 0);
  CHECK(select_first(/*start=*/0, 1_sym, 3_sym) == 1);
  CHECK(select_first(/*start=*/0, 2_sym, 3_sym) == 1);
  CHECK(select_first(/*start=*/0, 3_sym, 3_sym) == 5);

  CHECK(select_first(/*start=*/11, 0_sym, 3_sym) == 11);
  CHECK(select_first(/*start=*/11, 1_sym, 3_sym) == 13);
  CHECK(select_first(/*start=*/11, 2_sym, 3_sym) == 14);
  CHECK(select_first(/*start=*/11, 3_sym, 3_sym) == 20);

  SUBCASE("If no element satisfies the conditions returns index_npos") {
    CHECK(select_first(/*start=*/20, 0_sym, 0_sym) == index_npos);
    CHECK(select_first(/*start=*/21, 0_sym, 0_sym) == index_npos);
    CHECK(select_first(/*start=*/22, 0_sym, 0_sym) == index_npos);
    CHECK(select_first(/*start=*/23, 0_sym, 0_sym) == index_npos);
  }
}

TEST_CASE("[select_first] medium alphabet") {
  std::array<unsigned, 24> arr = {
      {0, 2, 2, 6, 5, 3, 6, 3, 2, 1, 3, 2, 0, 4, 5, 3, 7, 0, 0, 0, 3, 3, 2, 1}};

  int_vector vec(arr.size(), /*bpe=*/3);
  for (size_t i = 0; i < arr.size(); ++i) {
    vec[to_signed(i)] = arr[i];
  }

  const auto wt = wavelet_tree(vec);

  REQUIRE(wt.size() == 24);

  const auto select_first = [&](index_type start, symbol_id min,
                                symbol_id max) {
    return brwt::select_first(wt, start, brwt::between<symbol_id>{min, max});
  };

  // seq= 02265 3632 1320 4537 0003 321

  CHECK(select_first(/*start=*/0, 0_sym, 1_sym) == 0);
  CHECK(select_first(/*start=*/0, 1_sym, 1_sym) == 9);

  CHECK(select_first(/*start=*/0, 4_sym, 4_sym) == 13);

  CHECK(select_first(/*start=*/0, 5_sym, 7_sym) == 3);
  CHECK(select_first(/*start=*/0, 6_sym, 7_sym) == 3);
  CHECK(select_first(/*start=*/0, 7_sym, 7_sym) == 16);

  CHECK(select_first(/*start=*/9, 1_sym, 1_sym) == 9);
  CHECK(select_first(/*start=*/10, 1_sym, 1_sym) == 23);
  CHECK(select_first(/*start=*/11, 2_sym, 2_sym) == 11);
  CHECK(select_first(/*start=*/12, 2_sym, 2_sym) == 22);

  SUBCASE("If no element satisfies the conditions returns index_npos") {
    CHECK(select_first(/*start=*/17, 7_sym, 7_sym) == index_npos);
    CHECK(select_first(/*start=*/17, 6_sym, 7_sym) == index_npos);
    CHECK(select_first(/*start=*/17, 5_sym, 7_sym) == index_npos);
    CHECK(select_first(/*start=*/17, 4_sym, 7_sym) == index_npos);
    CHECK(select_first(/*start=*/17, 4_sym, 6_sym) == index_npos);
    CHECK(select_first(/*start=*/17, 4_sym, 5_sym) == index_npos);
    CHECK(select_first(/*start=*/17, 4_sym, 4_sym) == index_npos);

    CHECK(select_first(/*start=*/7, 6_sym, 6_sym) == index_npos);
    CHECK(select_first(/*start=*/8, 6_sym, 6_sym) == index_npos);
    CHECK(select_first(/*start=*/9, 6_sym, 6_sym) == index_npos);
    CHECK(select_first(/*start=*/10, 6_sym, 6_sym) == index_npos);
    CHECK(select_first(/*start=*/11, 6_sym, 6_sym) == index_npos);
    CHECK(select_first(/*start=*/12, 6_sym, 6_sym) == index_npos);
    CHECK(select_first(/*start=*/13, 6_sym, 6_sym) == index_npos);
  }
}
