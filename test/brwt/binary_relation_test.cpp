#include "doctest.h"
#include <brwt/binary_relation.h>

#include <algorithm>        // shuffle, for_each
#include <cassert>          // assert
#include <initializer_list> // initializer_list
#include <random>           // default_random_engine

using brwt::binary_relation;
using object_id = binary_relation::object_id;
using label_id = binary_relation::label_id;
using pair_type = binary_relation::pair_type;

static binary_relation make_test_binary_relation() {
  std::vector<pair_type> pairs;
  pairs.reserve(40);

  auto add_pairs = [&](const object_id object,
                       const std::initializer_list<label_id> labels) {
    std::for_each(begin(labels), end(labels), [&](const auto label) {
      pairs.push_back({object, label});
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
  add_pairs(0, {4, 8});
  add_pairs(1, {2, 4});
  add_pairs(2, {4});
  add_pairs(3, {2, 4, 6});
  add_pairs(4, {2, 4, 7, 8});
  add_pairs(5, {1, 3, 8, 9});
  add_pairs(6, {0, 2});
  add_pairs(7, {1, 3, 6, 8});
  add_pairs(8, {1, 3, 4, 7, 8, 9});
  add_pairs(9, {0, 2, 6, 7});
  add_pairs(10, {3, 4, 7, 9});
  add_pairs(11, {1, 2, 4, 8});

  assert(pairs.size() == 40);
  assert(std::none_of(begin(pairs), end(pairs),
                      [](const pair_type& p) { return p.label == 5; }));

  std::shuffle(begin(pairs), end(pairs), std::default_random_engine{});
  return binary_relation(pairs);
}

TEST_SUITE("binary_relation");

TEST_CASE("vector of pairs ctor") {
  std::vector<pair_type> pairs = {{0, 1}, {1, 2}, {0, 3}, {0, 4}};
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
  CHECK(binrel.rank(0, 0) == 0);
  CHECK(binrel.rank(0, 9) == 2);
  CHECK(binrel.rank(11, 0) == 2);

  CHECK(binrel.rank(1, 2) == 1);
  CHECK(binrel.rank(2, 3) == 1);
  CHECK(binrel.rank(3, 4) == 6);
  CHECK(binrel.rank(7, 6) == 16);
  CHECK(binrel.rank(8, 2) == 8);
  CHECK(binrel.rank(8, 3) == 11);

  CHECK(binrel.rank(7, 4) == 14);
  CHECK(binrel.rank(7, 5) == 14);
  CHECK(binrel.rank(7, 6) == 16);
  CHECK(binrel.rank(11, 4) == 24);
  CHECK(binrel.rank(11, 5) == 24);
  CHECK(binrel.rank(11, 6) == 27);

  CHECK(binrel.rank(10, 8) == 33);
  CHECK(binrel.rank(10, 9) == 36);
  CHECK(binrel.rank(11, 8) == 37);
  CHECK(binrel.rank(11, 9) == 40);
}

TEST_SUITE_END();
