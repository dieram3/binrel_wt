#include "doctest.h"

#include <brwt/bit_vector.h>
#include <brwt/bitmap.h>

#include <string> // string

using brwt::bitmap;
using brwt::bit_vector;

using size_type = bitmap::size_type;
using index_type = bitmap::index_type;

TEST_SUITE("bitmap");

TEST_CASE("bitmap::access()") {
  const std::vector<bit_vector> tests = {
      bit_vector{"10000101111"}, bit_vector{"10100110101111"},
      bit_vector{"11010111010111"}, bit_vector{"1101111111"},
  };

  for (const auto& test : tests) {
    auto bm = bitmap(test);

    CHECK(bm.length() == test.length());

    for (index_type i = 0; i < bm.length(); ++i) {
      CHECK(bm.access(i) == test.get(i));
    }
  }
}

TEST_CASE("bitmap::rank_1()") {
  std::vector<std::string> sequences = {
      "10100110101111",
      "10100110101111010011010111101001101011110100110101111010011010111101001"
      "00110101110",
      "10100110101111010011010111101001101011110100111101001101011110100",
      "10100110101111010011010111101001101011110100111101001101011110101010"
      "01101011110100110101111010011010111101001111010011010111101010100110"
      "10111101001101011110100110101111010011110100110101111010101001101011"
      "11010011010111101001101011110100111101001101011110101010011010111101"
      "00110101111010011010111101001111010011010111101000000101001101011110"
      "10011010111101001101011110100111101001101011110100",
      "11010011010111101001101011110100110101111010011110100110101111010101"
      "01010011010111101001101011110100110101111010011110100110101111010101"
      "01010011010111101001101011110100110101111010011110100110101111010101"
      "01010011010111101001101011110100110101111010011110100110101111010101"
      "01010011010111101001101011110100110101111010011110100110101111010101"
      "01010011010111101001101011110100110101111010011110100110101111010101"
      "01010011010111101001101011110100110101111010011110100110101111010101"
      "01010011010111101001101011110100110101111010011110100110101111010101"
      "01010011010111101001101011110100110101111010011110100110101111010101"
      "01010011010111101001101011110100110101111010011110100110101111010101"
      "01010011010111101001101011110100110101111010011110100110101111010101"
      "01010011010111101001101011110100110101111010011110100110101111010101"
      "00100110101111010011010111101001101011110100111101001101011110101010"
      "10100110101111010011010111101001101011110100111101001101011110101010"
      "10100110101111010011010111101001101011110100111101001101011110101010"
      "10100110101111010011010111101001101011110100111101001101011110101010"
      "10100110101111010011010111101001101011110100111101001101011110101010"
      "10100110101111010011010111101001101011110100111101001101011110101010"
      "10100110101111010011010111101001101011110100111101001101011110101010"
      "10100110101111010011010111101001101011110100111101001101011110101010"
      "10100110101111010011010111101001101011110100111101001101011110101010"
      "10100110101111010011010111101001101011110100111101001101011110101010"
      "10100110101111010011010111101001101011110100111101001101011110101010"
      "1010011010111101001101011110100110101111010011110100110101111010101"};

  const auto bm0 = bitmap(bit_vector(sequences[0]));
  const auto bm1 = bitmap(bit_vector(sequences[1]));
  const auto bm2 = bitmap(bit_vector(sequences[2]));
  const auto bm3 = bitmap(bit_vector(sequences[3]));
  const auto bm4 = bitmap(bit_vector(sequences[4]));

  CHECK(bm0.rank_1(0) == 1);
  CHECK(bm0.rank_1(1) == 2);
  CHECK(bm0.rank_1(2) == 3);
  CHECK(bm0.rank_1(3) == 4);
  CHECK(bm0.rank_1(4) == 4);
  CHECK(bm0.rank_1(5) == 5);

  CHECK(bm1.rank_1(0) == 0);
  CHECK(bm1.rank_1(1) == 1);
  CHECK(bm1.rank_1(2) == 2);
  CHECK(bm1.rank_1(3) == 3);
  CHECK(bm1.rank_1(4) == 3);
  CHECK(bm1.rank_1(11) == 7);
  CHECK(bm1.rank_1(37) == 23);
  CHECK(bm1.rank_1(64) == 39);

  CHECK(bm2.rank_1(0) == 0);
  CHECK(bm2.rank_1(22) == 13);
  CHECK(bm2.rank_1(38) == 23);
  CHECK(bm2.rank_1(57) == 35);
  CHECK(bm2.rank_1(64) == 39);

  CHECK(bm3.rank_1(31) == 19);
  CHECK(bm3.rank_1(316) == 190);
  CHECK(bm3.rank_1(382) == 230);
  CHECK(bm3.rank_1(389) == 234);

  CHECK(bm4.rank_1(681) == 411);
  CHECK(bm4.rank_1(1144) == 691);
  CHECK(bm4.rank_1(1630) == 984);
}

TEST_CASE("bitmap::rank_0()") {
  const std::vector<std::string> sequences = {"10100110101111",
                                              "10100110101110"};

  const auto bm0 = bitmap(bit_vector(sequences[0]));
  const auto bm1 = bitmap(bit_vector(sequences[1]));

  CHECK(bm0.rank_0(0) == 0);
  CHECK(bm0.rank_0(1) == 0);
  CHECK(bm0.rank_0(2) == 0);
  CHECK(bm0.rank_0(3) == 0);
  CHECK(bm0.rank_0(4) == 1);
  CHECK(bm0.rank_0(5) == 1);
  CHECK(bm0.rank_0(6) == 2);
  CHECK(bm0.rank_0(9) == 3);
  CHECK(bm0.rank_0(10) == 4);
  CHECK(bm0.rank_0(12) == 5);

  CHECK(bm1.rank_0(0) == 1);
  CHECK(bm1.rank_0(4) == 2);
  CHECK(bm1.rank_0(8) == 3);
  CHECK(bm1.rank_0(9) == 4);
  CHECK(bm1.rank_0(10) == 5);
  CHECK(bm1.rank_0(11) == 5);
  CHECK(bm1.rank_0(12) == 6);
}

TEST_CASE("bitmap::select_1()") {
  const std::vector<std::string> sequences = {
      "10100110101111",
      "10100110101111010011010111101001101011110100110101111010011010111101001"
      "00110101110",
      "10100110101111010011010111101001101011110100111101001101011110101010011"
      "01011110100110101111010011010111101001111010011010111101010100110101111"
      "01001101011110100110101111010011110100110101111010101001101011110100110"
      "10111101001101011110100111101001101011110101010011010111101001101011110"
      "10011010111101001111010011010111101000000101001101011110100110101111010"
      "01101011110100111101001101011110100",
      "11010011010111101001101011110100110101111010011110100110101111010101010"
      "10011010111101001101011110100110101111010011110100110101111010101010100"
      "11010111101001101011110100110101111010011110100110101111010101010100110"
      "10111101001101011110100110101111010011110100110101111010101010100110101"
      "11101001101011110100110101111010011110100110101111010101010100110101111"
      "01001101011110100110101111010011110100110101111010101010100110101111010"
      "01101011110100110101111010011110100110101111010100101001101011110100110"
      "10111101001101011110100111101001101011110101010011010111101001101011110"
      "10011010111101001111010011010111101010100100110101111010011010111101001"
      "10101111010011110100110101111010101010100110101111010011010111101001101"
      "01111010011110100110101111010101010100110101111010011010111101001101011"
      "11010011110100110101111010101010100110101111010011010111101001101011110"
      "10011110100110101111010101010100110101111010011010111101001101011110100"
      "11110100110101111010101010100110101111010011010111101001101011110100111"
      "10100110101111010101010100110101111010011010111101001101011110100111101"
      "00110101111010101010100110101111010011010111101001101011110100111101001"
      "10101111010101010100110101111010011010111101001101011110100111101001101"
      "01111010101010100110101111010011010111101001101011110100111101001101011"
      "11010101010100110101111010011010111101001101011110100111101001101011110"
      "10101010100110101111010011010111101001101011110100111101001101011110101"
      "01"};

  const auto bm0 = bitmap(bit_vector(sequences[0]));
  const auto bm1 = bitmap(bit_vector(sequences[1]));
  const auto bm2 = bitmap(bit_vector(sequences[2]));
  const auto bm3 = bitmap(bit_vector(sequences[3]));

  const auto bm4 = bitmap(bit_vector("11111"));
  const auto bm5 = bitmap(bit_vector("00000"));

  SUBCASE("Valid queries") {
    CHECK(bm0.select_1(1) == 0);
    CHECK(bm0.select_1(2) == 1);
    CHECK(bm0.select_1(3) == 2);
    CHECK(bm0.select_1(4) == 3);
    CHECK(bm0.select_1(5) == 5);
    CHECK(bm0.select_1(6) == 7);
    CHECK(bm0.select_1(7) == 8);
    CHECK(bm0.select_1(8) == 11);
    CHECK(bm0.select_1(9) == 13);

    CHECK(bm1.select_1(7) == 11);
    CHECK(bm1.select_1(23) == 37);
    CHECK(bm1.select_1(39) == 63);

    CHECK(bm2.select_1(19) == 30);
    CHECK(bm2.select_1(150) == 250);
    CHECK(bm2.select_1(230) == 381);
    CHECK(bm2.select_1(234) == 389);

    CHECK(bm3.select_1(560) == 926);
    CHECK(bm3.select_1(670) == 1110);
    CHECK(bm3.select_1(700) == 1159);
    CHECK(bm3.select_1(701) == 1161);
    CHECK(bm3.select_1(702) == 1163);
    CHECK(bm3.select_1(703) == 1164);
    CHECK(bm3.select_1(800) == 1326);
    CHECK(bm3.select_1(804) == 1331);
    CHECK(bm3.select_1(850) == 1408);

    CHECK(bm4.select_1(1) == 0);
    CHECK(bm4.select_1(2) == 1);
    CHECK(bm4.select_1(3) == 2);
    CHECK(bm4.select_1(4) == 3);
    CHECK(bm4.select_1(5) == 4);
  }

  SUBCASE("Number of ones is less than the query") {
    CHECK(bm0.select_1(10) == -1);
    CHECK(bm0.select_1(11) == -1);
    CHECK(bm0.select_1(12) == -1);
    CHECK(bm0.select_1(13) == -1);

    CHECK(bm1.select_1(50) == -1);
    CHECK(bm1.select_1(60) == -1);
    CHECK(bm1.select_1(70) == -1);
    CHECK(bm1.select_1(80) == -1);

    CHECK(bm3.select_1(860) == -1);
    CHECK(bm3.select_1(870) == -1);
    CHECK(bm3.select_1(880) == -1);
    CHECK(bm3.select_1(900) == -1);

    CHECK(bm5.select_1(1) == -1);
    CHECK(bm5.select_1(2) == -1);
    CHECK(bm5.select_1(3) == -1);
    CHECK(bm5.select_1(4) == -1);
    CHECK(bm5.select_1(5) == -1);
  }
}

TEST_CASE("bitmap::select_0()") {
  const std::vector<std::string> sequences = {
      "10100110101111",
      "11010011010111101001101011110100110101111010011110100110101111010101010"
      "10011010111101001101011110100110101111010011110100110101111010101010100"
      "11010111101001101011110100110101111010011110100110101111010101010100110"
      "10111101001101011110100110101111010011110100110101111010101010100110101"
      "11101001101011110100110101111010011110100110101111010101010100110101111"
      "01001101011110100110101111010011110100110101111010101010100110101111010"
      "01101011110100110101111010011110100110101111010100101001101011110100110"
      "10111101001101011110100111101001101011110101010011010111101001101011110"
      "10011010111101001111010011010111101010100100110101111010011010111101001"
      "10101111010011110100110101111010101010100110101111010011010111101001101"
      "01111010011110100110101111010101010100110101111010011010111101001101011"
      "11010011110100110101111010101010100110101111010011010111101001101011110"
      "10011110100110101111010101010100110101111010011010111101001101011110100"
      "11110100110101111010101010100110101111010011010111101001101011110100111"
      "10100110101111010101010100110101111010011010111101001101011110100111101"
      "00110101111010101010100110101111010011010111101001101011110100111101001"
      "10101111010101010100110101111010011010111101001101011110100111101001101"
      "01111010101010100110101111010011010111101001101011110100111101001101011"
      "11010101010100110101111010011010111101001101011110100111101001101011110"
      "10101010100110101111010011010111101001101011110100111101001101011110101"
      "01",
  };

  const auto bm0 = bitmap(bit_vector(sequences[0]));
  const auto bm1 = bitmap(bit_vector(sequences[1]));

  const auto bm2 = bitmap(bit_vector("00000"));
  const auto bm3 = bitmap(bit_vector("11111"));

  SUBCASE("Valid queries") {
    CHECK(bm0.select_0(1) == 4);
    CHECK(bm0.select_0(2) == 6);
    CHECK(bm0.select_0(3) == 9);
    CHECK(bm0.select_0(4) == 10);
    CHECK(bm0.select_0(5) == 12);

    CHECK(bm1.select_0(430) == 1081);
    CHECK(bm1.select_0(431) == 1083);
    CHECK(bm1.select_0(499) == 1255);
    CHECK(bm1.select_0(500) == 1257);
    CHECK(bm1.select_0(503) == 1267);
    CHECK(bm1.select_0(504) == 1268);
    CHECK(bm1.select_0(560) == 1411);

    CHECK(bm2.select_0(1) == 0);
    CHECK(bm2.select_0(2) == 1);
    CHECK(bm2.select_0(3) == 2);
    CHECK(bm2.select_0(4) == 3);
    CHECK(bm2.select_0(5) == 4);
  }

  SUBCASE("Number of zeros is less than the query") {
    CHECK(bm0.select_0(6) == -1);
    CHECK(bm0.select_0(7) == -1);
    CHECK(bm0.select_0(8) == -1);
    CHECK(bm0.select_0(9) == -1);
    CHECK(bm0.select_0(10) == -1);
    CHECK(bm0.select_0(11) == -1);
    CHECK(bm0.select_0(12) == -1);
    CHECK(bm0.select_0(13) == -1);

    CHECK(bm1.select_0(600) == -1);
    CHECK(bm1.select_0(700) == -1);
    CHECK(bm1.select_0(1255) == -1);
    CHECK(bm1.select_0(1411) == -1);

    CHECK(bm3.select_0(1) == -1);
    CHECK(bm3.select_0(2) == -1);
    CHECK(bm3.select_0(3) == -1);
    CHECK(bm3.select_0(4) == -1);
    CHECK(bm3.select_0(5) == -1);
  }
}
