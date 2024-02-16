#include <doctest/doctest.h>

import brwt.utility;

using brwt::ceil_div;

// TEST_SUITE("utility");

TEST_CASE("ceil_div") {
  SUBCASE("When remainder is 0") {
    CHECK(ceil_div(10, 5) == 2);
    CHECK(ceil_div(2, 1) == 2);
    CHECK(ceil_div(19278, 714) == 27);
    CHECK(ceil_div(19278, 27) == 714);
  }
  SUBCASE("When remainder is not 0") {
    CHECK(ceil_div(12, 5) == 3);
    CHECK(ceil_div(5, 2) == 3);
    CHECK(ceil_div(100, 11) == 10);

    REQUIRE(1321239 / 12314 == 107);
    CHECK(ceil_div(1321239, 12314) == 108);
  }

  // check constexpr
  static_assert(ceil_div(5, 2) == 3);
  static_assert(ceil_div(10UL, 3UL) == 4UL);

  // check noexcept
  static_assert(noexcept(ceil_div(1, 1)));
  static_assert(noexcept(ceil_div(1UL, 1UL)));
}

TEST_SUITE_END();
