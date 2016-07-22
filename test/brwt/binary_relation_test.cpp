#include "doctest.h"
#include <brwt/binary_relation.h>

#include <algorithm>        // shuffle, for_each
#include <cassert>          // assert
#include <initializer_list> // initializer_list
#include <ostream>          // ostream
#include <random>           // default_random_engine
#include <type_traits>      // underlying_type_t, is_enum

// TODO(Diego): Adapt the test data as objects with no associated pairs
// are allowed now :)

using brwt::binary_relation;
using brwt::nullopt;
using brwt::index_type;
using object_id = binary_relation::object_id;
using label_id = binary_relation::label_id;
using pair_type = binary_relation::pair_type;

static object_id operator"" _obj(unsigned long long x) {
  return static_cast<object_id>(x);
}

static label_id operator"" _lab(unsigned long long x) {
  return static_cast<label_id>(x);
}

static pair_type pair(const object_id object, const label_id label) {
  return pair_type{object, label};
}

static binary_relation make_test_binary_relation() {
  std::vector<pair_type> pairs;
  pairs.reserve(40);

  auto add_pairs = [&](const object_id object,
                       const std::initializer_list<unsigned> labels) {
    std::for_each(begin(labels), end(labels), [&](const auto label) {
      pairs.push_back({object, static_cast<label_id>(label)});
    });
  };

  // In the following matrix, objects are rows and labels are columns.
  // For now, it is required that each object_id has at least one associated
  // pair.
  // Conversely, labels with no associated pair are allowed.

  // The returned binary relation will have the following points.
  //    |0|1|2|3|4|5|6|7|8|9|
  //  0 |_|_|_|_|x|_|_|_|x|_|
  //  1 |_|_|x|_|x|_|_|_|_|_|
  //  2 |_|_|_|_|x|_|_|_|_|_|
  //  3 |_|_|x|_|x|_|x|_|_|_|
  //  4 |_|_|x|_|x|_|_|x|x|_|
  //  5 |_|x|_|x|_|_|_|_|x|x|
  //  6 |x|_|x|_|_|_|_|_|_|_|
  //  7 |_|x|_|x|_|_|x|_|x|_|
  //  8 |_|x|_|x|x|_|_|x|x|x|
  //  9 |x|_|x|_|_|_|x|x|_|_|
  // 10 |_|_|_|x|x|_|_|x|_|x|
  // 11 |_|x|x|_|x|_|_|_|x|_|
  //
  // Intentionally, no pair has label_id = 5
  add_pairs(0_obj, {4, 8});
  add_pairs(1_obj, {2, 4});
  add_pairs(2_obj, {4});
  add_pairs(3_obj, {2, 4, 6});
  add_pairs(4_obj, {2, 4, 7, 8});
  add_pairs(5_obj, {1, 3, 8, 9});
  add_pairs(6_obj, {0, 2});
  add_pairs(7_obj, {1, 3, 6, 8});
  add_pairs(8_obj, {1, 3, 4, 7, 8, 9});
  add_pairs(9_obj, {0, 2, 6, 7});
  add_pairs(10_obj, {3, 4, 7, 9});
  add_pairs(11_obj, {1, 2, 4, 8});

  assert(pairs.size() == 40);
  assert(std::none_of(begin(pairs), end(pairs),
                      [](const pair_type& p) { return p.label == 5_lab; }));

  std::shuffle(begin(pairs), end(pairs), std::default_random_engine{});
  return binary_relation(pairs);
}

namespace brwt {
template <typename T>
static constexpr auto to_underlying_type(const T value) {
  static_assert(std::is_enum<T>::value, "");
  return static_cast<std::underlying_type_t<T>>(value);
}

static std::ostream& operator<<(std::ostream& os, const object_id value) {
  return os << to_underlying_type(value);
}

static std::ostream& operator<<(std::ostream& os, const label_id value) {
  return os << to_underlying_type(value);
}

static std::ostream& operator<<(std::ostream& os, const pair_type p) {
  return os << '(' << p.object << ", " << p.label << ')';
}

template <typename T>
static std::ostream& operator<<(std::ostream& os, const optional<T>& opt) {
  if (opt) {
    return os << *opt;
  }
  return os << "nullopt";
}

} // end namespace brwt

TEST_SUITE("binary_relation");

TEST_CASE("vector of pairs ctor") {
  std::vector<pair_type> pairs = {
      {0_obj, 1_lab}, {1_obj, 2_lab}, {0_obj, 3_lab}, {0_obj, 4_lab},
  };
  binary_relation binrel(pairs); // does not hang
  CHECK(binrel.size() == 4);
  CHECK(binrel.num_objects() == 2);
}

TEST_CASE("size and num_objects") {
  const auto binrel = make_test_binary_relation();
  CHECK(binrel.size() == 40);
  CHECK(binrel.num_objects() == 12);
}

TEST_CASE("Rank with max object, max label") {
  const auto binrel = make_test_binary_relation();
  CHECK(binrel.rank(0_obj, 0_lab) == 0);
  CHECK(binrel.rank(0_obj, 9_lab) == 2);
  CHECK(binrel.rank(11_obj, 0_lab) == 2);

  CHECK(binrel.rank(1_obj, 2_lab) == 1);
  CHECK(binrel.rank(2_obj, 3_lab) == 1);
  CHECK(binrel.rank(3_obj, 4_lab) == 6);
  CHECK(binrel.rank(7_obj, 6_lab) == 16);
  CHECK(binrel.rank(8_obj, 2_lab) == 8);
  CHECK(binrel.rank(8_obj, 3_lab) == 11);

  CHECK(binrel.rank(7_obj, 4_lab) == 14);
  CHECK(binrel.rank(7_obj, 5_lab) == 14);
  CHECK(binrel.rank(7_obj, 6_lab) == 16);
  CHECK(binrel.rank(11_obj, 4_lab) == 24);
  CHECK(binrel.rank(11_obj, 5_lab) == 24);
  CHECK(binrel.rank(11_obj, 6_lab) == 27);

  CHECK(binrel.rank(10_obj, 8_lab) == 33);
  CHECK(binrel.rank(10_obj, 9_lab) == 36);
  CHECK(binrel.rank(11_obj, 8_lab) == 37);
  CHECK(binrel.rank(11_obj, 9_lab) == 40);
}

TEST_CASE("Rank with min object, max object, max label") {
  const auto br = make_test_binary_relation();
  CHECK(br.rank(0_obj, 0_obj, 0_lab) == 0);
  CHECK(br.rank(0_obj, 8_obj, 0_lab) == 1);
  CHECK(br.rank(0_obj, 9_obj, 0_lab) == 2);

  CHECK(br.rank(0_obj, 0_obj, 5_lab) == 1);
  CHECK(br.rank(4_obj, 4_obj, 7_lab) == 3);
  CHECK(br.rank(8_obj, 8_obj, 4_lab) == 3);
  CHECK(br.rank(11_obj, 11_obj, 8_lab) == 4);

  CHECK(br.rank(0_obj, 0_obj, 9_lab) == 2);
  CHECK(br.rank(4_obj, 4_obj, 9_lab) == 4);
  CHECK(br.rank(8_obj, 8_obj, 9_lab) == 6);
  CHECK(br.rank(11_obj, 11_obj, 9_lab) == 4);

  CHECK(br.rank(4_obj, 5_obj, 4_lab) == 4);
  CHECK(br.rank(4_obj, 6_obj, 4_lab) == 6);
  CHECK(br.rank(2_obj, 7_obj, 6_lab) == 13);
  CHECK(br.rank(2_obj, 7_obj, 7_lab) == 14);

  CHECK(br.rank(7_obj, 10_obj, 6_lab) == 11);
  CHECK(br.rank(7_obj, 10_obj, 8_lab) == 16);
  CHECK(br.rank(7_obj, 10_obj, 9_lab) == 18);
  CHECK(br.rank(7_obj, 11_obj, 6_lab) == 14);
  CHECK(br.rank(7_obj, 11_obj, 8_lab) == 20);
  CHECK(br.rank(7_obj, 11_obj, 9_lab) == 22);

  CHECK(br.rank(3_obj, 11_obj, 9_lab) == 35);
  CHECK(br.rank(2_obj, 11_obj, 9_lab) == 36);
  CHECK(br.rank(1_obj, 11_obj, 9_lab) == 38);
  CHECK(br.rank(0_obj, 11_obj, 9_lab) == 40);
}

static auto make_label_major_nth_element() {
  return [br = make_test_binary_relation()](const auto... args) {
    using brwt::lab_major;
    return br.nth_element(args..., lab_major);
  };
}

// ==========================================
// n-th element in label major order
// ==========================================

TEST_CASE("[nth_element,lab_major]: Line queries") {
  const auto nth_element = make_label_major_nth_element();

  // first line.
  CHECK(nth_element(0_obj, 0_obj, 0_lab, 1) == pair(0_obj, 4_lab));
  CHECK(nth_element(0_obj, 0_obj, 0_lab, 2) == pair(0_obj, 8_lab));
  CHECK(nth_element(0_obj, 0_obj, 0_lab, 3) == nullopt);
  CHECK(nth_element(0_obj, 0_obj, 4_lab, 1) == pair(0_obj, 4_lab));
  CHECK(nth_element(0_obj, 0_obj, 5_lab, 1) == pair(0_obj, 8_lab));
  CHECK(nth_element(0_obj, 0_obj, 8_lab, 1) == pair(0_obj, 8_lab));

  // random lines
  CHECK(nth_element(2_obj, 2_obj, 2_lab, 1) == pair(2_obj, 4_lab));
  CHECK(nth_element(5_obj, 5_obj, 0_lab, 2) == pair(5_obj, 3_lab));
  CHECK(nth_element(8_obj, 8_obj, 3_lab, 3) == pair(8_obj, 7_lab));
  CHECK(nth_element(10_obj, 10_obj, 6_lab, 2) == pair(10_obj, 9_lab));
}

TEST_CASE("[nth_element,lab_major]: Last pairs and out of range") {
  const auto nth_element = make_label_major_nth_element();

  CHECK(nth_element(2_obj, 6_obj, 0_lab, 14) == pair(5_obj, 9_lab));
  CHECK(nth_element(2_obj, 6_obj, 0_lab, 15) == nullopt);
  CHECK(nth_element(2_obj, 6_obj, 5_lab, 130) == nullopt);
  CHECK(nth_element(2_obj, 6_obj, 5_lab, 5) == pair(5_obj, 9_lab));
  CHECK(nth_element(2_obj, 6_obj, 5_lab, 6) == nullopt);
  CHECK(nth_element(2_obj, 6_obj, 5_lab, 192) == nullopt);

  CHECK(nth_element(4_obj, 9_obj, 0_lab, 24) == pair(8_obj, 9_lab));
  CHECK(nth_element(4_obj, 9_obj, 0_lab, 25) == nullopt);
  CHECK(nth_element(4_obj, 9_obj, 7_lab, 9) == pair(8_obj, 9_lab));
  CHECK(nth_element(4_obj, 9_obj, 7_lab, 10) == nullopt);

  CHECK(nth_element(0_obj, 3_obj, 0_lab, 8) == pair(0_obj, 8_lab));
  CHECK(nth_element(0_obj, 3_obj, 2_lab, 8) == pair(0_obj, 8_lab));
  CHECK(nth_element(0_obj, 3_obj, 2_lab, 9) == nullopt);

  CHECK(nth_element(6_obj, 11_obj, 0_lab, 24) == pair(10_obj, 9_lab));
  CHECK(nth_element(6_obj, 11_obj, 3_lab, 16) == pair(10_obj, 9_lab));
  CHECK(nth_element(6_obj, 11_obj, 3_lab, 17) == nullopt);

  CHECK(nth_element(0_obj, 11_obj, 0_lab, 40) == pair(10_obj, 9_lab));
  CHECK(nth_element(0_obj, 11_obj, 0_lab, 41) == nullopt);
  CHECK(nth_element(0_obj, 11_obj, 0_lab, 2323) == nullopt);
  CHECK(nth_element(0_obj, 11_obj, 4_lab, 24) == pair(10_obj, 9_lab));
  CHECK(nth_element(0_obj, 11_obj, 4_lab, 25) == nullopt);
  CHECK(nth_element(0_obj, 11_obj, 4_lab, 5343) == nullopt);
}

TEST_CASE("[nth_element,lab_major]: Objects ranges") {
  const auto nth_element = make_label_major_nth_element();

  // range of objects [6, 8]
  CHECK(nth_element(6_obj, 8_obj, 0_lab, 1) == pair(6_obj, 0_lab));
  CHECK(nth_element(6_obj, 8_obj, 0_lab, 2) == pair(7_obj, 1_lab));
  CHECK(nth_element(6_obj, 8_obj, 0_lab, 3) == pair(8_obj, 1_lab));
  CHECK(nth_element(6_obj, 8_obj, 0_lab, 7) == pair(8_obj, 4_lab));
  CHECK(nth_element(6_obj, 8_obj, 0_lab, 8) == pair(7_obj, 6_lab));
  CHECK(nth_element(6_obj, 8_obj, 0_lab, 12) == pair(8_obj, 9_lab));
  CHECK(nth_element(6_obj, 8_obj, 4_lab, 1) == pair(8_obj, 4_lab));
  CHECK(nth_element(6_obj, 8_obj, 4_lab, 2) == pair(7_obj, 6_lab));
  CHECK(nth_element(6_obj, 8_obj, 4_lab, 6) == pair(8_obj, 9_lab));

  // Random ranges
  CHECK(nth_element(2_obj, 8_obj, 3_lab, 7) == pair(8_obj, 4_lab));
  CHECK(nth_element(4_obj, 10_obj, 5_lab, 13) == pair(10_obj, 9_lab));
  CHECK(nth_element(5_obj, 11_obj, 3_lab, 15) == pair(8_obj, 8_lab));
  CHECK(nth_element(0_obj, 5_obj, 2_lab, 4) == pair(5_obj, 3_lab));
  CHECK(nth_element(1_obj, 9_obj, 3_lab, 17) == pair(7_obj, 8_lab));
  CHECK(nth_element(0_obj, 11_obj, 4_lab, 21) == pair(11_obj, 8_lab));

  // Full range
  CHECK(nth_element(0_obj, 11_obj, 0_lab, 1) == pair(6_obj, 0_lab));
  CHECK(nth_element(0_obj, 11_obj, 0_lab, 2) == pair(9_obj, 0_lab));
  CHECK(nth_element(0_obj, 11_obj, 0_lab, 3) == pair(5_obj, 1_lab));
  CHECK(nth_element(0_obj, 11_obj, 0_lab, 10) == pair(6_obj, 2_lab));
  CHECK(nth_element(0_obj, 11_obj, 0_lab, 20) == pair(3_obj, 4_lab));
  CHECK(nth_element(0_obj, 11_obj, 0_lab, 30) == pair(9_obj, 7_lab));
  CHECK(nth_element(0_obj, 11_obj, 0_lab, 38) == pair(5_obj, 9_lab));
  CHECK(nth_element(0_obj, 11_obj, 0_lab, 39) == pair(8_obj, 9_lab));
  CHECK(nth_element(0_obj, 11_obj, 0_lab, 40) == pair(10_obj, 9_lab));
}

// ==========================================
// n-th element in object major order
// ==========================================

static auto make_object_major_nth_element() {
  return [br = make_test_binary_relation()](const auto... args) {
    using brwt::obj_major;
    return br.nth_element(args..., obj_major);
  };
}

TEST_CASE("[nth_element,obj_major]: One column test") {
  const auto nth_element = make_object_major_nth_element();

  CHECK(nth_element(0_obj, 0_lab, 0_lab, 1) == pair(6_obj, 0_lab));
  CHECK(nth_element(0_obj, 0_lab, 0_lab, 2) == pair(9_obj, 0_lab));
  CHECK(nth_element(0_obj, 0_lab, 0_lab, 3) == nullopt);
  CHECK(nth_element(0_obj, 2_lab, 2_lab, 1) == pair(1_obj, 2_lab));
  CHECK(nth_element(0_obj, 2_lab, 2_lab, 6) == pair(11_obj, 2_lab));
  CHECK(nth_element(0_obj, 2_lab, 2_lab, 7) == nullopt);
  CHECK(nth_element(0_obj, 5_lab, 5_lab, 1) == nullopt);
  CHECK(nth_element(0_obj, 8_lab, 8_lab, 1) == pair(0_obj, 8_lab));
  CHECK(nth_element(0_obj, 8_lab, 8_lab, 6) == pair(11_obj, 8_lab));
  CHECK(nth_element(0_obj, 8_lab, 8_lab, 7) == nullopt);
  CHECK(nth_element(0_obj, 9_lab, 9_lab, 1) == pair(5_obj, 9_lab));
  CHECK(nth_element(0_obj, 9_lab, 9_lab, 2) == pair(8_obj, 9_lab));
  CHECK(nth_element(0_obj, 9_lab, 9_lab, 3) == pair(10_obj, 9_lab));
  CHECK(nth_element(0_obj, 9_lab, 9_lab, 4) == nullopt);

  CHECK(nth_element(5_obj, 0_lab, 0_lab, 1) == pair(6_obj, 0_lab));
  CHECK(nth_element(6_obj, 0_lab, 0_lab, 1) == pair(6_obj, 0_lab));
  CHECK(nth_element(7_obj, 0_lab, 0_lab, 1) == pair(9_obj, 0_lab));
  CHECK(nth_element(10_obj, 0_lab, 0_lab, 1) == nullopt);
  CHECK(nth_element(5_obj, 5_lab, 5_lab, 1) == nullopt);
  CHECK(nth_element(11_obj, 5_lab, 5_lab, 1) == nullopt);
  CHECK(nth_element(5_obj, 9_lab, 9_lab, 1) == pair(5_obj, 9_lab));
  CHECK(nth_element(6_obj, 9_lab, 9_lab, 1) == pair(8_obj, 9_lab));

  CHECK(nth_element(5_obj, 8_lab, 8_lab, 1) == pair(5_obj, 8_lab));
  CHECK(nth_element(5_obj, 8_lab, 8_lab, 2) == pair(7_obj, 8_lab));
  CHECK(nth_element(5_obj, 8_lab, 8_lab, 3) == pair(8_obj, 8_lab));
  CHECK(nth_element(5_obj, 8_lab, 8_lab, 4) == pair(11_obj, 8_lab));
  CHECK(nth_element(5_obj, 8_lab, 8_lab, 5) == nullopt);
  CHECK(nth_element(5_obj, 8_lab, 8_lab, 42) == nullopt);
}

TEST_CASE("[nth_element,obj_major]: Left range test") {
  const auto nth_element = make_object_major_nth_element();

  CHECK(nth_element(0_obj, 0_lab, 4_lab, 1) == pair(0_obj, 4_lab));
  CHECK(nth_element(0_obj, 0_lab, 4_lab, 2) == pair(1_obj, 2_lab));
  CHECK(nth_element(0_obj, 0_lab, 4_lab, 12) == pair(6_obj, 2_lab));
  CHECK(nth_element(0_obj, 0_lab, 4_lab, 23) == pair(11_obj, 2_lab));
  CHECK(nth_element(0_obj, 0_lab, 4_lab, 24) == pair(11_obj, 4_lab));
  CHECK(nth_element(0_obj, 0_lab, 4_lab, 25) == nullopt);
  CHECK(nth_element(0_obj, 0_lab, 4_lab, 42) == nullopt);

  CHECK(nth_element(6_obj, 0_lab, 4_lab, 1) == pair(6_obj, 0_lab));
  CHECK(nth_element(6_obj, 0_lab, 4_lab, 2) == pair(6_obj, 2_lab));
  CHECK(nth_element(6_obj, 0_lab, 4_lab, 9) == pair(9_obj, 2_lab));
  CHECK(nth_element(6_obj, 0_lab, 4_lab, 13) == pair(11_obj, 2_lab));
  CHECK(nth_element(6_obj, 0_lab, 4_lab, 14) == pair(11_obj, 4_lab));
  CHECK(nth_element(6_obj, 0_lab, 4_lab, 15) == nullopt);
  CHECK(nth_element(6_obj, 0_lab, 4_lab, 42) == nullopt);

  CHECK(nth_element(11_obj, 0_lab, 4_lab, 1) == pair(11_obj, 1_lab));
  CHECK(nth_element(11_obj, 0_lab, 4_lab, 2) == pair(11_obj, 2_lab));
  CHECK(nth_element(11_obj, 0_lab, 4_lab, 3) == pair(11_obj, 4_lab));
  CHECK(nth_element(11_obj, 0_lab, 4_lab, 4) == nullopt);
  CHECK(nth_element(11_obj, 0_lab, 4_lab, 42) == nullopt);
}

TEST_CASE("[nth_element,obj_major]: Right range test") {
  const auto nth_element = make_object_major_nth_element();

  CHECK(nth_element(0_obj, 5_lab, 9_lab, 1) == pair(0_obj, 8_lab));
  CHECK(nth_element(0_obj, 5_lab, 9_lab, 2) == pair(3_obj, 6_lab));
  CHECK(nth_element(0_obj, 5_lab, 9_lab, 10) == pair(8_obj, 8_lab));
  CHECK(nth_element(0_obj, 5_lab, 9_lab, 15) == pair(10_obj, 9_lab));
  CHECK(nth_element(0_obj, 5_lab, 9_lab, 16) == pair(11_obj, 8_lab));
  CHECK(nth_element(0_obj, 5_lab, 9_lab, 17) == nullopt);
  CHECK(nth_element(0_obj, 5_lab, 9_lab, 42) == nullopt);

  CHECK(nth_element(6_obj, 5_lab, 9_lab, 1) == pair(7_obj, 6_lab));
  CHECK(nth_element(6_obj, 5_lab, 9_lab, 2) == pair(7_obj, 8_lab));
  CHECK(nth_element(6_obj, 5_lab, 9_lab, 6) == pair(9_obj, 6_lab));
  CHECK(nth_element(6_obj, 5_lab, 9_lab, 9) == pair(10_obj, 9_lab));
  CHECK(nth_element(6_obj, 5_lab, 9_lab, 10) == pair(11_obj, 8_lab));
  CHECK(nth_element(6_obj, 5_lab, 9_lab, 11) == nullopt);
  CHECK(nth_element(6_obj, 5_lab, 9_lab, 42) == nullopt);

  CHECK(nth_element(11_obj, 5_lab, 9_lab, 1) == pair(11_obj, 8_lab));
  CHECK(nth_element(11_obj, 5_lab, 9_lab, 2) == nullopt);
  CHECK(nth_element(11_obj, 5_lab, 9_lab, 42) == nullopt);
}

TEST_CASE("[nth_element,obj_major]: Center range test") {
  const auto nth_element = make_object_major_nth_element();

  CHECK(nth_element(0_obj, 2_lab, 7_lab, 1) == pair(0_obj, 4_lab));
  CHECK(nth_element(0_obj, 2_lab, 7_lab, 2) == pair(1_obj, 2_lab));
  CHECK(nth_element(0_obj, 2_lab, 7_lab, 12) == pair(6_obj, 2_lab));
  CHECK(nth_element(0_obj, 2_lab, 7_lab, 24) == pair(11_obj, 2_lab));
  CHECK(nth_element(0_obj, 2_lab, 7_lab, 25) == pair(11_obj, 4_lab));
  CHECK(nth_element(0_obj, 2_lab, 7_lab, 26) == nullopt);
  CHECK(nth_element(0_obj, 2_lab, 7_lab, 42) == nullopt);

  CHECK(nth_element(6_obj, 2_lab, 7_lab, 1) == pair(6_obj, 2_lab));
  CHECK(nth_element(6_obj, 2_lab, 7_lab, 2) == pair(7_obj, 3_lab));
  CHECK(nth_element(6_obj, 2_lab, 7_lab, 6) == pair(8_obj, 7_lab));
  CHECK(nth_element(6_obj, 2_lab, 7_lab, 13) == pair(11_obj, 2_lab));
  CHECK(nth_element(6_obj, 2_lab, 7_lab, 14) == pair(11_obj, 4_lab));
  CHECK(nth_element(6_obj, 2_lab, 7_lab, 15) == nullopt);
  CHECK(nth_element(6_obj, 2_lab, 7_lab, 42) == nullopt);

  CHECK(nth_element(11_obj, 2_lab, 7_lab, 1) == pair(11_obj, 2_lab));
  CHECK(nth_element(11_obj, 2_lab, 7_lab, 2) == pair(11_obj, 4_lab));
  CHECK(nth_element(11_obj, 2_lab, 7_lab, 3) == nullopt);
  CHECK(nth_element(11_obj, 2_lab, 7_lab, 42) == nullopt);
}

TEST_CASE("[nth_element,obj_major]: Tiny range test") {
  const auto nth_element = make_object_major_nth_element();

  CHECK(nth_element(0_obj, 5_lab, 7_lab, 1) == pair(3_obj, 6_lab));
  CHECK(nth_element(0_obj, 5_lab, 7_lab, 2) == pair(4_obj, 7_lab));
  CHECK(nth_element(0_obj, 5_lab, 7_lab, 3) == pair(7_obj, 6_lab));
  CHECK(nth_element(0_obj, 5_lab, 7_lab, 4) == pair(8_obj, 7_lab));
  CHECK(nth_element(0_obj, 5_lab, 7_lab, 5) == pair(9_obj, 6_lab));
  CHECK(nth_element(0_obj, 5_lab, 7_lab, 6) == pair(9_obj, 7_lab));
  CHECK(nth_element(0_obj, 5_lab, 7_lab, 7) == pair(10_obj, 7_lab));
  CHECK(nth_element(0_obj, 5_lab, 7_lab, 8) == nullopt);
  CHECK(nth_element(0_obj, 5_lab, 7_lab, 42) == nullopt);

  CHECK(nth_element(6_obj, 5_lab, 7_lab, 1) == pair(7_obj, 6_lab));
  CHECK(nth_element(6_obj, 5_lab, 7_lab, 2) == pair(8_obj, 7_lab));
  CHECK(nth_element(6_obj, 5_lab, 7_lab, 3) == pair(9_obj, 6_lab));
  CHECK(nth_element(6_obj, 5_lab, 7_lab, 4) == pair(9_obj, 7_lab));
  CHECK(nth_element(6_obj, 5_lab, 7_lab, 5) == pair(10_obj, 7_lab));
  CHECK(nth_element(6_obj, 5_lab, 7_lab, 6) == nullopt);
  CHECK(nth_element(6_obj, 5_lab, 7_lab, 42) == nullopt);

  CHECK(nth_element(11_obj, 5_lab, 7_lab, 1) == nullopt);
  CHECK(nth_element(11_obj, 5_lab, 7_lab, 2) == nullopt);
  CHECK(nth_element(11_obj, 5_lab, 7_lab, 3) == nullopt);
  CHECK(nth_element(11_obj, 5_lab, 7_lab, 42) == nullopt);
}

TEST_CASE("[nth_element,obj_major]: Full range test") {
  const auto nth_element = make_object_major_nth_element();

  CHECK(nth_element(0_obj, 0_lab, 9_lab, 1) == pair(0_obj, 4_lab));
  CHECK(nth_element(0_obj, 0_lab, 9_lab, 2) == pair(0_obj, 8_lab));
  CHECK(nth_element(0_obj, 0_lab, 9_lab, 8) == pair(3_obj, 6_lab));
  CHECK(nth_element(0_obj, 0_lab, 9_lab, 24) == pair(8_obj, 3_lab));
  CHECK(nth_element(0_obj, 0_lab, 9_lab, 31) == pair(9_obj, 6_lab));
  CHECK(nth_element(0_obj, 0_lab, 9_lab, 39) == pair(11_obj, 4_lab));
  CHECK(nth_element(0_obj, 0_lab, 9_lab, 40) == pair(11_obj, 8_lab));
  CHECK(nth_element(0_obj, 0_lab, 9_lab, 41) == nullopt);
  CHECK(nth_element(0_obj, 0_lab, 9_lab, 42) == nullopt);
  CHECK(nth_element(0_obj, 0_lab, 9_lab, 3141) == nullopt);

  CHECK(nth_element(6_obj, 0_lab, 9_lab, 1) == pair(6_obj, 0_lab));
  CHECK(nth_element(6_obj, 0_lab, 9_lab, 2) == pair(6_obj, 2_lab));
  CHECK(nth_element(6_obj, 0_lab, 9_lab, 10) == pair(8_obj, 7_lab));
  CHECK(nth_element(6_obj, 0_lab, 9_lab, 23) == pair(11_obj, 4_lab));
  CHECK(nth_element(6_obj, 0_lab, 9_lab, 24) == pair(11_obj, 8_lab));
  CHECK(nth_element(6_obj, 0_lab, 9_lab, 25) == nullopt);
  CHECK(nth_element(6_obj, 0_lab, 9_lab, 42) == nullopt);
  CHECK(nth_element(6_obj, 0_lab, 9_lab, 3141) == nullopt);

  CHECK(nth_element(11_obj, 0_lab, 9_lab, 1) == pair(11_obj, 1_lab));
  CHECK(nth_element(11_obj, 0_lab, 9_lab, 2) == pair(11_obj, 2_lab));
  CHECK(nth_element(11_obj, 0_lab, 9_lab, 3) == pair(11_obj, 4_lab));
  CHECK(nth_element(11_obj, 0_lab, 9_lab, 4) == pair(11_obj, 8_lab));
  CHECK(nth_element(11_obj, 0_lab, 9_lab, 5) == nullopt);
  CHECK(nth_element(11_obj, 0_lab, 9_lab, 42) == nullopt);
  CHECK(nth_element(11_obj, 0_lab, 9_lab, 3141) == nullopt);
}

// ==========================================
// Count distinct labels tests
// ==========================================

TEST_CASE("count_distinct_labels, basic") {
  const auto br = make_test_binary_relation();
  // Rectangle starting from (0, 0)
  CHECK(br.count_distinct_labels(0_obj, 0_obj, 0_lab, 0_lab) == 0);
  CHECK(br.count_distinct_labels(0_obj, 4_obj, 0_lab, 4_lab) == 2);
  CHECK(br.count_distinct_labels(0_obj, 8_obj, 0_lab, 8_lab) == 8);
  CHECK(br.count_distinct_labels(0_obj, 11_obj, 0_lab, 9_lab) == 9);

  // Rows
  CHECK(br.count_distinct_labels(0_obj, 0_obj, 0_lab, 9_lab) == 2);
  CHECK(br.count_distinct_labels(5_obj, 5_obj, 0_lab, 9_lab) == 4);
  CHECK(br.count_distinct_labels(8_obj, 8_obj, 0_lab, 9_lab) == 6);
  CHECK(br.count_distinct_labels(11_obj, 11_obj, 0_lab, 9_lab) == 4);

  // Columns
  CHECK(br.count_distinct_labels(0_obj, 11_obj, 0_lab, 0_lab) == 1);
  CHECK(br.count_distinct_labels(0_obj, 11_obj, 2_lab, 2_lab) == 1);
  CHECK(br.count_distinct_labels(0_obj, 11_obj, 4_lab, 4_lab) == 1);
  CHECK(br.count_distinct_labels(0_obj, 11_obj, 5_lab, 5_lab) == 0);
  CHECK(br.count_distinct_labels(0_obj, 11_obj, 6_lab, 6_lab) == 1);
  CHECK(br.count_distinct_labels(0_obj, 11_obj, 9_lab, 9_lab) == 1);

  // Spread (all labels)
  CHECK(br.count_distinct_labels(0_obj, 3_obj, 0_lab, 9_lab) == 4);
  CHECK(br.count_distinct_labels(3_obj, 7_obj, 0_lab, 9_lab) == 9);
  CHECK(br.count_distinct_labels(9_obj, 11_obj, 0_lab, 9_lab) == 9);
  CHECK(br.count_distinct_labels(10_obj, 11_obj, 0_lab, 9_lab) == 7);
}

TEST_CASE("count_distinct_labels, complex") {
  const auto br = make_test_binary_relation();
  // Different wavelet tree nodes:
  CHECK(br.count_distinct_labels(1_obj, 3_obj, 0_lab, 7_lab) == 3);
  CHECK(br.count_distinct_labels(1_obj, 3_obj, 0_lab, 3_lab) == 1);
  CHECK(br.count_distinct_labels(1_obj, 3_obj, 4_lab, 7_lab) == 2);
  CHECK(br.count_distinct_labels(1_obj, 3_obj, 0_lab, 1_lab) == 0);
  CHECK(br.count_distinct_labels(1_obj, 3_obj, 2_lab, 3_lab) == 1);
  CHECK(br.count_distinct_labels(1_obj, 3_obj, 4_lab, 5_lab) == 1);
  CHECK(br.count_distinct_labels(1_obj, 3_obj, 6_lab, 7_lab) == 1);

  // Random label pairs
  auto count_labels = [&](const label_id alpha, const label_id beta) {
    return br.count_distinct_labels(1_obj, 4_obj, alpha, beta);
  };
  CHECK(count_labels(1_lab, 6_lab) == 3);
  CHECK(count_labels(2_lab, 9_lab) == 5);
  CHECK(count_labels(3_lab, 8_lab) == 4);
  CHECK(count_labels(0_lab, 5_lab) == 2);
  CHECK(count_labels(3_lab, 5_lab) == 1);
  CHECK(count_labels(7_lab, 9_lab) == 2);
  CHECK(count_labels(1_lab, 8_lab) == 5);
  CHECK(count_labels(0_lab, 5_lab) == 2);
  CHECK(count_labels(3_lab, 9_lab) == 4);
  CHECK(count_labels(4_lab, 8_lab) == 4);
}

TEST_SUITE_END();
