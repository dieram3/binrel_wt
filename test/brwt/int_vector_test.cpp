#include "doctest.h"
#include <brwt/int_vector.h>

#include <limits>      // numeric_limits
#include <stdexcept>   // domain_error
#include <type_traits> // is_signed, is_unsigned, ...

using brwt::int_vector;
// bpe stands for 'bits per element'

static_assert(std::is_unsigned<int_vector::value_type>::value, "");
static_assert(std::is_signed<int_vector::size_type>::value, "");

static_assert(std::is_nothrow_default_constructible<int_vector>::value, "");
static_assert(std::is_copy_constructible<int_vector>::value, "");
static_assert(std::is_copy_assignable<int_vector>::value, "");
static_assert(std::is_nothrow_move_constructible<int_vector>::value, "");
static_assert(std::is_nothrow_move_assignable<int_vector>::value, "");
static_assert(std::is_nothrow_destructible<int_vector>::value, "");

TEST_SUITE("int_vector");

TEST_CASE("int_vector::int_vector()") {
  const int_vector v{};
  CHECK(v.size() == 0);
  CHECK(v.get_bpe() == 0);
  CHECK(v.allocated_bytes() == 0);
}

TEST_CASE("int_vector::int_vector(size_type, int)") {
  const int_vector v(/*count=*/10, /*bpe=*/3);
  CHECK(v.size() == 10);
  CHECK(v.get_bpe() == 3);
  CHECK(v.allocated_bytes() >= 8);

  for (int i = 0; i < v.size(); ++i) {
    CHECK(v[i] == 0);
  }

  // bits per block
  constexpr auto bpb = std::numeric_limits<int_vector::value_type>::digits;

  CHECK_NOTHROW(int_vector(10, bpb - 1));
  CHECK_THROWS_AS(int_vector(10, bpb), std::domain_error);     // NOLINT
  CHECK_THROWS_AS(int_vector(10, bpb + 1), std::domain_error); // NOLINT
  CHECK_THROWS_AS(int_vector(10, bpb + 2), std::domain_error); // NOLINT
}

TEST_CASE("int_vector::set_value") {
  SUBCASE("No invalid values") {
    int_vector vec(/*count=*/100, /*bpe=*/10);
    // The maximum storable value is 1023.

    for (int i = 0; i < vec.size(); ++i) {
      vec[i] = static_cast<unsigned>(i * 10);
    }

    CHECK(vec[0] == 0);
    CHECK(vec[1] == 10);
    CHECK(vec[2] == 20);
    CHECK(vec[99] == 990);
    CHECK(vec[98] == 980);
    CHECK(vec[31] == 310);

    CHECK(vec[44] == 440);
    CHECK(vec[45] == 450);
    CHECK(vec[46] == 460);

    vec[45] = 1023;

    CHECK(vec[44] == 440);
    CHECK(vec[45] == 1023);
    CHECK(vec[46] == 460);

    static_assert(noexcept(vec[0] = 140), "");
  }
  SUBCASE("There are invalid values") {
    int_vector vec(/*count=*/50, /*bpe=*/9);
    // The maximum storable value is 511

    vec[30] = 511;

    CHECK(vec[29] == 0);
    CHECK(vec[30] == 511);
    CHECK(vec[31] == 0);

    CHECK(vec[29] == 0);
    CHECK(vec[30] == 511);
    CHECK(vec[31] == 0);

    CHECK(vec[40] == 0);

    vec[40] = 500;

    CHECK(vec[39] == 0);
    CHECK(vec[40] == 500);
    CHECK(vec[41] == 0);
  }
}

TEST_CASE("int_vector::size") {
  CHECK(int_vector(10, 14).size() == 10);
  CHECK(int_vector(10, 41).size() == 10);
  CHECK(int_vector(0, 14).size() == 0);
  CHECK(int_vector(42, 0).size() == 42);
  CHECK(int_vector(42, 25).size() == 42);
}

TEST_CASE("int_vector::get_bpe()") {
  CHECK(int_vector(10, 14).get_bpe() == 14);
  CHECK(int_vector(30, 14).get_bpe() == 14);
  CHECK(int_vector(0, 12).get_bpe() == 12);
  CHECK(int_vector(21, 0).get_bpe() == 0);
  CHECK(int_vector(1, 32).get_bpe() == 32);
}

TEST_CASE("int_vector::allocated_bytes()") {
  CHECK(int_vector(10, 14).allocated_bytes() >= 24);
  CHECK(int_vector(0, 15).allocated_bytes() == 0);
  CHECK(int_vector(19, 0).allocated_bytes() == 0);
  CHECK(int_vector(0, 0).allocated_bytes() == 0);
  CHECK(int_vector(41, 37).allocated_bytes() >= 192);
}

TEST_SUITE_END();
