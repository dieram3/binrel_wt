#include "doctest.h"
#include <brwt/bit_hacks.h>

#include <cstdint> // uint64_t

/// TODO(Diego): Test the count_leading_zeros overloads.
/// TODO(Diego): Test the count_trailing_zeros overloads.
/// TODO(Diego): Test used_bits

using uint = unsigned int;
using ulong = unsigned long;
using ullong = unsigned long long;

TEST_SUITE("bit_hacks");

TEST_CASE("pop_count") {
  using brwt::pop_count;

  SUBCASE("unsigned overload") {
    CHECK(pop_count(0x1111u) == 4);
    CHECK(pop_count(0x1212u) == 4);
    CHECK(pop_count(0x1F2Fu) == 10);
    CHECK(pop_count(0xFFFFu) == 16);
  }
  SUBCASE("unsigned long overload") {
    CHECK(pop_count(0x1111'1111ul) == 8);
    CHECK(pop_count(0x1214'2814ul) == 8);
    CHECK(pop_count(0x12F2'439Eul) == 15);
    CHECK(pop_count(0xFFFF'FFFFul) == 32);
  }
  SUBCASE("unsigned long long overload") {
    CHECK(pop_count(0x1111'1111'1111'1111ull) == 16);
    CHECK(pop_count(0x1248'1428'1144'2488ull) == 16);
    CHECK(pop_count(0x14C0'2491'A9B3'2390ull) == 23);
    CHECK(pop_count(0xFFFF'FFFF'FFFF'FFFFull) == 64);
  }

  // check constexpr
  static_assert(pop_count(0x1111u) == 4, "");
  static_assert(pop_count(0x12F2'439Eul) == 15, "");
  static_assert(pop_count(0x14C0'2491'A9B3'2390ull) == 23, "");
  // check noexcept
  static_assert(noexcept(pop_count(uint{})), "");
  static_assert(noexcept(pop_count(ulong{})), "");
  static_assert(noexcept(pop_count(ullong{})), "");
}

TEST_CASE("lsb_mask") {
  using brwt::lsb_mask;
  using std::uint64_t;

  CHECK(lsb_mask<unsigned>(1) == 0b0000'0001);
  CHECK(lsb_mask<unsigned>(3) == 0b0000'0111);
  CHECK(lsb_mask<unsigned>(5) == 0b0001'1111);
  CHECK(lsb_mask<unsigned>(7) == 0b0111'1111);

  CHECK(lsb_mask<uint64_t>(62) == 0x3FFF'FFFF'FFFF'FFFF);
  CHECK(lsb_mask<uint64_t>(63) == 0x7FFF'FFFF'FFFF'FFFF);

  // check constexpr
  static_assert(lsb_mask<uint>(8) == 0xFF, "");
  static_assert(lsb_mask<ulong>(8) == 0xFF, "");
  static_assert(lsb_mask<ullong>(8) == 0xFF, "");

  // check noexcept
  static_assert(noexcept(lsb_mask<uint>(4)), "");
  static_assert(noexcept(lsb_mask<ulong>(4)), "");
  static_assert(noexcept(lsb_mask<ullong>(4)), "");
}

TEST_SUITE_END();
