#include "doctest.h"
#include <brwt/wavelet_tree.h>

#include <brwt/int_vector.h> // int_vector
#include <array>             // array
#include <ostream>           // ostream
#include <vector>            // vector

using brwt::wavelet_tree;
using brwt::int_vector;
using std::size_t;
using std::ptrdiff_t;
using symbol_type = wavelet_tree::symbol_type;

static constexpr symbol_type map_upper(const char c) noexcept {
  return static_cast<symbol_type>(c - 'A');
}

// Creates an int_vector with 2 bits per element
static int_vector create_vector_with_2_bpe() {
  std::array<unsigned, 24> arr = {
      {0, 2, 2, 1, 2, 3, 1, 3, 2, 1, 3, 0, 0, 1, 2, 0, 1, 0, 0, 0, 3, 3, 2, 1}};

  int_vector vec(arr.size(), /*bpe=*/2);
  for (size_t i = 0; i < arr.size(); ++i) {
    vec[static_cast<ptrdiff_t>(i)] = arr[i];
  }

  return vec;
}

static int_vector create_vector_with_3_bpe() {
  const std::string str = "EHDHACEEGBCBGCF";
  int_vector vec(static_cast<ptrdiff_t>(str.size()), /*bpe=*/3);

  for (size_t i = 0; i < str.size(); ++i) {
    vec[static_cast<ptrdiff_t>(i)] = map_upper(str[i]);
  }

  return vec;
}

static auto to_std_vector(const int_vector& vec) {
  std::vector<symbol_type> res(static_cast<size_t>(vec.length()));
  for (size_t i = 0; i < res.size(); ++i) {
    res[i] = vec[static_cast<ptrdiff_t>(i)];
  }
  return res;
}

static auto to_std_vector(const wavelet_tree& wt) {
  std::vector<symbol_type> res(static_cast<size_t>(wt.length()));
  for (size_t i = 0; i < res.size(); ++i) {
    res[i] = wt.access(static_cast<ptrdiff_t>(i));
  }
  return res;
}

namespace std {
static std::ostream& operator<<(std::ostream& os,
                                const std::vector<symbol_type>& vec) {
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

TEST_CASE("Constructor from int_vector") {
  {
    wavelet_tree wt(create_vector_with_2_bpe());
    CHECK(wt.length() == 24);
    CHECK(wt.get_alphabet_size() == 4);
  }
  {
    wavelet_tree wt(create_vector_with_3_bpe());
    CHECK(wt.length() == 15);
    CHECK(wt.get_alphabet_size() == 8);
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
  REQUIRE(wt.length() == 24);

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
  REQUIRE(wt.length() == 15);

  auto rank = [&](const char c, const wavelet_tree::size_type pos) {
    return wt.rank(map_upper(c), pos);
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

TEST_SUITE_END();
