#include "doctest.h"
#include <brwt/bit_hacks.h>

#include <cstdint> // uint64_t

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
}

TEST_SUITE_END();
