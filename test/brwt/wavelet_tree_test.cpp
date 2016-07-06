#include "doctest.h"
#include <brwt/wavelet_tree.h>

#include <brwt/int_vector.h> // int_vector
#include <array>             // array
#include <cstddef>           // size_t, ptrdiff_t
#include <ostream>           // ostream
#include <string>            // string
#include <type_traits>       // is_nothrow_default_constructible, ...
#include <vector>            // vector

using brwt::wavelet_tree;
using brwt::int_vector;
using std::size_t;
using std::ptrdiff_t;
using symbol_id = wavelet_tree::symbol_id;

static constexpr symbol_id map_upper(const char c) {
  return static_cast<symbol_id>(c - 'A');
}

static constexpr auto to_unsigned(const ptrdiff_t value) {
  return static_cast<size_t>(value);
}
static constexpr auto to_signed(const size_t value) {
  return static_cast<ptrdiff_t>(value);
}

// Creates an int_vector with 2 bits per element
static int_vector create_vector_with_2_bpe() {
  std::array<unsigned, 24> arr = {
      {0, 2, 2, 1, 2, 3, 1, 3, 2, 1, 3, 0, 0, 1, 2, 0, 1, 0, 0, 0, 3, 3, 2, 1}};

  int_vector vec(arr.size(), /*bpe=*/2);
  for (size_t i = 0; i < arr.size(); ++i) {
    vec[to_signed(i)] = arr[i];
  }

  return vec;
}

static int_vector create_vector_with_3_bpe() {
  const std::string str = "EHDHACEEGBCBGCF";
  int_vector vec(to_signed(str.size()), /*bpe=*/3);

  for (size_t i = 0; i < str.size(); ++i) {
    using value_t = int_vector::value_type;
    const auto value = static_cast<value_t>(map_upper(str[i]));
    vec[to_signed(i)] = value;
  }

  return vec;
}

static auto to_std_vector(const int_vector& vec) {
  std::vector<symbol_id> res(to_unsigned(vec.length()));
  for (size_t i = 0; i < res.size(); ++i) {
    res[i] = vec[to_signed(i)];
  }
  return res;
}

static auto to_std_vector(const wavelet_tree& wt) {
  std::vector<symbol_id> res(to_unsigned(wt.size()));
  for (size_t i = 0; i < res.size(); ++i) {
    res[i] = wt.access(to_signed(i));
  }
  return res;
}

static auto to_string(const wavelet_tree::node_desc& node) {
  std::string str(to_unsigned(node.size()), '\0');
  for (size_t i = 0; i < str.size(); ++i) {
    str[i] = node.access(to_signed(i)) ? '1' : '0';
  }
  return str;
}

namespace std {
static std::ostream& operator<<(std::ostream& os,
                                const std::vector<symbol_id>& vec) {
  os << '\n';
  os << '[';
  for (size_t i = 0; i + 1 < vec.size(); ++i) {
    os << vec[i] << ", ";
  }
  if (!vec.empty()) {
    os << vec.back();
  }
  os << ']';
  return os;
}
} // end namespace std

TEST_SUITE("wavelet_tree");

template <typename T>
static void test_default_member_functions() {
  static_assert(std::is_nothrow_default_constructible<T>::value, "");
  static_assert(std::is_copy_constructible<T>::value, "");
  static_assert(std::is_copy_assignable<T>::value, "");
  static_assert(std::is_nothrow_move_constructible<T>::value, "");
  static_assert(std::is_nothrow_move_assignable<T>::value, "");
  static_assert(std::is_nothrow_destructible<T>::value, "");
}

TEST_CASE("Default constructor") {
  const wavelet_tree wt{};
  CHECK(wt.size() == 0);
  CHECK(wt.get_bits_per_symbol() == 0);
  CHECK(wt.max_symbol_id() == 0);

  // static asserts for the wavelet tree go here.
  test_default_member_functions<wavelet_tree>();
}

TEST_CASE("Constructor from int_vector") {
  {
    const wavelet_tree wt(create_vector_with_2_bpe());
    CHECK(wt.size() == 24);
    CHECK(wt.get_bits_per_symbol() == 2);
    CHECK(wt.max_symbol_id() == 3);
  }
  {
    const wavelet_tree wt(create_vector_with_3_bpe());
    CHECK(wt.size() == 15);
    CHECK(wt.get_bits_per_symbol() == 3);
    CHECK(wt.max_symbol_id() == 7);
  }
}

TEST_CASE("Access with sigma=4") {
  const auto vec = create_vector_with_2_bpe();
  const auto wt = wavelet_tree(vec);
  // Explicit tests
  CHECK(wt.access(1) == 2);
  CHECK(wt.access(7) == 3);
  CHECK(wt.access(19) == 0);
  // Non-explicit full range check
  CHECK(to_std_vector(wt) == to_std_vector(vec));
}

TEST_CASE("Access with sigma=8") {
  auto vec = create_vector_with_3_bpe();
  const auto wt = wavelet_tree(vec);
  // Explicit tests
  CHECK(wt.access(1) == map_upper('H'));
  CHECK(wt.access(6) == map_upper('E'));
  CHECK(wt.access(14) == map_upper('F'));
  // Non-explicit full range check
  CHECK(to_std_vector(wt) == to_std_vector(vec));
}

TEST_CASE("Rank with sigma=4") {
  const auto wt = wavelet_tree(create_vector_with_2_bpe());
  // seq = 0221 2313 2130 0120 1000 3321
  REQUIRE(wt.size() == 24);

  CHECK(wt.rank(/*symbol=*/0, /*pos=*/0) == 1);
  CHECK(wt.rank(/*symbol=*/0, /*pos=*/8) == 1);
  CHECK(wt.rank(/*symbol=*/0, /*pos=*/11) == 2);
  CHECK(wt.rank(/*symbol=*/0, /*pos=*/13) == 3);
  CHECK(wt.rank(/*symbol=*/0, /*pos=*/23) == 7);

  CHECK(wt.rank(/*symbol=*/1, /*pos=*/0) == 0);
  CHECK(wt.rank(/*symbol=*/1, /*pos=*/5) == 1);
  CHECK(wt.rank(/*symbol=*/1, /*pos=*/12) == 3);
  CHECK(wt.rank(/*symbol=*/1, /*pos=*/13) == 4);
  CHECK(wt.rank(/*symbol=*/1, /*pos=*/23) == 6);

  CHECK(wt.rank(/*symbol=*/2, /*pos=*/0) == 0);
  CHECK(wt.rank(/*symbol=*/2, /*pos=*/4) == 3);
  CHECK(wt.rank(/*symbol=*/2, /*pos=*/12) == 4);
  CHECK(wt.rank(/*symbol=*/2, /*pos=*/22) == 6);
  CHECK(wt.rank(/*symbol=*/2, /*pos=*/23) == 6);

  CHECK(wt.rank(/*symbol=*/3, /*pos=*/0) == 0);
  CHECK(wt.rank(/*symbol=*/3, /*pos=*/4) == 0);
  CHECK(wt.rank(/*symbol=*/3, /*pos=*/5) == 1);
  CHECK(wt.rank(/*symbol=*/3, /*pos=*/17) == 3);
  CHECK(wt.rank(/*symbol=*/3, /*pos=*/23) == 5);
}

TEST_CASE("Rank with sigma=8") {
  const auto wt = wavelet_tree(create_vector_with_3_bpe());
  // seq = EHDHA CEEGB CBGCF
  REQUIRE(wt.size() == 15);

  auto rank = [&](const char symbol, const wavelet_tree::size_type pos) {
    return wt.rank(map_upper(symbol), pos);
  };

  CHECK(rank('A', 0) == 0);
  CHECK(rank('C', 0) == 0);
  CHECK(rank('E', 0) == 1);
  CHECK(rank('G', 0) == 0);

  CHECK(rank('B', 5) == 0);
  CHECK(rank('B', 8) == 0);
  CHECK(rank('B', 9) == 1);
  CHECK(rank('B', 10) == 1);
  CHECK(rank('B', 11) == 2);

  CHECK(rank('C', 4) == 0);
  CHECK(rank('C', 5) == 1);
  CHECK(rank('E', 10) == 3);
  CHECK(rank('G', 10) == 1);

  CHECK(rank('A', 14) == 1);
  CHECK(rank('B', 14) == 2);
  CHECK(rank('C', 14) == 3);
  CHECK(rank('D', 14) == 1);
  CHECK(rank('E', 14) == 3);
  CHECK(rank('F', 14) == 1);
  CHECK(rank('G', 14) == 2);
  CHECK(rank('H', 14) == 2);
}

TEST_CASE("Select with sigma=4") {
  const auto wt = wavelet_tree(create_vector_with_2_bpe());
  // seq = 0221 2313 2130 0120 1000 3321
  REQUIRE(wt.size() == 24);

  // Random queries
  CHECK(wt.select(/*symbol=*/0, /*nth=*/4) == 15);
  CHECK(wt.select(/*symbol=*/2, /*nth=*/3) == 4);
  CHECK(wt.select(/*symbol=*/1, /*nth=*/2) == 6);
  CHECK(wt.select(/*symbol=*/3, /*nth=*/3) == 10);
  CHECK(wt.select(/*symbol=*/2, /*nth=*/4) == 8);
  CHECK(wt.select(/*symbol=*/1, /*nth=*/5) == 16);
  CHECK(wt.select(/*symbol=*/0, /*nth=*/4) == 15);
  CHECK(wt.select(/*symbol=*/3, /*nth=*/1) == 5);

  // Last occurrences
  CHECK(wt.select(/*symbol=*/0, /*nth=*/7) == 19);
  CHECK(wt.select(/*symbol=*/1, /*nth=*/6) == 23);
  CHECK(wt.select(/*symbol=*/2, /*nth=*/6) == 22);
  CHECK(wt.select(/*symbol=*/3, /*nth=*/5) == 21);

  // Barely out of range.
  CHECK(wt.select(/*symbol=*/0, /*nth=*/8) == -1);
  CHECK(wt.select(/*symbol=*/1, /*nth=*/7) == -1);
  CHECK(wt.select(/*symbol=*/2, /*nth=*/7) == -1);
  CHECK(wt.select(/*symbol=*/3, /*nth=*/6) == -1);

  // Totally out of range.
  CHECK(wt.select(/*symbol=*/0, /*nth=*/190) == -1);
  CHECK(wt.select(/*symbol=*/1, /*nth=*/1312) == -1);
  CHECK(wt.select(/*symbol=*/2, /*nth=*/122) == -1);
  CHECK(wt.select(/*symbol=*/3, /*nth=*/423) == -1);
}

TEST_CASE("Select with sigma=8") {
  const auto wt = wavelet_tree(create_vector_with_3_bpe());
  // seq = EHDHA CEEGB CBGCF
  REQUIRE(wt.size() == 15);

  auto select = [&](const char symbol, const wavelet_tree::size_type pos) {
    return wt.select(map_upper(symbol), pos);
  };

  CHECK(select('A', 1) == 4);
  CHECK(select('A', 2) == -1);

  CHECK(select('B', 1) == 9);
  CHECK(select('B', 2) == 11);
  CHECK(select('B', 3) == -1);

  CHECK(select('C', 1) == 5);
  CHECK(select('C', 2) == 10);
  CHECK(select('C', 3) == 13);
  CHECK(select('C', 4) == -1);

  CHECK(select('D', 1) == 2);
  CHECK(select('D', 2) == -1);

  CHECK(select('E', 1) == 0);
  CHECK(select('E', 2) == 6);
  CHECK(select('E', 3) == 7);
  CHECK(select('E', 4) == -1);

  CHECK(select('F', 1) == 14);
  CHECK(select('F', 2) == -1);

  CHECK(select('G', 1) == 8);
  CHECK(select('G', 2) == 12);
  CHECK(select('G', 3) == -1);

  CHECK(select('H', 1) == 1);
  CHECK(select('H', 2) == 3);
  CHECK(select('H', 3) == -1);

  // Totally out of range.
  CHECK(select('A', 4) == -1);
  CHECK(select('B', 13) == -1);
  CHECK(select('E', 74) == -1);
  CHECK(select('H', 9923) == -1);
}

TEST_CASE("size, get_bits_per_symbol, max_symbol_id") {
  {
    const auto wt = wavelet_tree(int_vector(34, /*bpe=*/8));
    CHECK(wt.size() == 34);
    CHECK(wt.get_bits_per_symbol() == 8);
    CHECK(wt.max_symbol_id() == 255);
  }
  {
    const auto wt = wavelet_tree(int_vector(411, /*bpe=*/10));
    CHECK(wt.size() == 411);
    CHECK(wt.get_bits_per_symbol() == 10);
    CHECK(wt.max_symbol_id() == 1023);
  }
}

TEST_CASE("Navigation in WT with sigma=4") {
  const auto wt = wavelet_tree(create_vector_with_2_bpe());
  // seq = 022123132130012010003321

  const auto root = wt.make_root();
  const auto node_0 = root.make_lhs();
  const auto node_1 = root.make_rhs();

  CHECK(!root.is_leaf());
  CHECK(node_0.is_leaf());
  CHECK(node_1.is_leaf());

  CHECK(to_string(root) == "011011011010001000001110");
  CHECK(to_string(node_0) == "0111001010001");
  CHECK(to_string(node_1) == "00011010110");
}

TEST_CASE("Navigation in WT with sigma=8") {
  const auto wt = wavelet_tree(create_vector_with_3_bpe());
  // seq = EHDHACEEGBCBGCF

  const auto root = wt.make_root();
  const auto node_0 = root.make_lhs();
  const auto node_1 = root.make_rhs();
  const auto node_00 = node_0.make_lhs();
  const auto node_01 = node_0.make_rhs();
  const auto node_10 = node_1.make_lhs();
  const auto node_11 = node_1.make_rhs();

  CHECK(!root.is_leaf());
  CHECK(!node_0.is_leaf());
  CHECK(!node_1.is_leaf());
  CHECK(node_00.is_leaf());
  CHECK(node_01.is_leaf());
  CHECK(node_10.is_leaf());
  CHECK(node_11.is_leaf());

  CHECK(to_string(root) == "110100111000101");
  CHECK(to_string(node_0) == "1010101");
  CHECK(to_string(node_1) == "01100110");
  CHECK(to_string(node_00) == "011");
  CHECK(to_string(node_01) == "1000");
  CHECK(to_string(node_10) == "0001");
  CHECK(to_string(node_11) == "1100");
}

TEST_SUITE_END();
