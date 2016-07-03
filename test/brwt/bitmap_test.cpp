#include "doctest.h"
#include <brwt/bit_vector.h>
#include <brwt/bitmap.h>

#include <algorithm> // min
#include <ostream>   // string

using brwt::bitmap;
using brwt::bit_vector;

using size_type = bitmap::size_type;
using index_type = bitmap::index_type;

TEST_SUITE("bit_vector");

TEST_CASE("bitmap::access()") {
  std::vector<std::string> tests = {
      "10000101111", "10100110101111", "11010111010111", "1101111111",
  };

  for (const auto& test : tests) {
    auto vec = bit_vector(test);
    auto bm = bitmap(vec);

    auto len = vec.length();

    for (size_type i = 0; i < len; ++i) {
      CHECK(bm.access(static_cast<index_type>(i)) == vec.get(i));
    }
  }
}

TEST_CASE("bitmap::rank_1()") {
  struct expectation {
    index_type pos;
    size_type count;
  };

  struct test_case {
    std::string sequence;
    std::vector<expectation> expectations;
  };

  std::vector<test_case> tests = {
      {"10100110101111", {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 4}, {5, 5}}},
      {"10100110101110", {{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 3}, {5, 4}}},
      {"10100110101111010011010111101001101011110100110101111010011010111101001"
       "00110101110",
       {{0, 0}, {11, 7}, {64, 39}, {37, 23}}},
      {"10100110101111010011010111101001101011110100111101001101011110100",
       {{0, 0}, {64, 39}, {22, 13}, {38, 23}, {57, 35}}},
      {"10100110101111010011010111101001101011110100111101001101011110101010"
       "01101011110100110101111010011010111101001111010011010111101010100110"
       "10111101001101011110100110101111010011110100110101111010101001101011"
       "11010011010111101001101011110100111101001101011110101010011010111101"
       "00110101111010011010111101001111010011010111101000000101001101011110"
       "10011010111101001101011110100111101001101011110100",
       {{390, 234}, {382, 230}, {316, 190}, {31, 19}}},
      {"11010011010111101001101011110100110101111010011110100110101111010101"
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
       "1010011010111101001101011110100110101111010011110100110101111010101",
       {{1632, 984}, {1144, 691}, {681, 411}}}};

  for (const auto& test : tests) {
    auto vec = bit_vector(test.sequence);
    auto bm = bitmap(vec);
    for (const auto& exp : test.expectations) {
      CHECK(bm.rank_1(exp.pos) == exp.count);
    }
  }
}

TEST_CASE("bitmap::rank_0()") {
  struct expectation {
    index_type pos;
    size_type count;
  };

  struct test_case {
    std::string sequence;
    std::vector<expectation> expectations;
  };

  std::vector<test_case> tests = {
      {"10100110101111", {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 1}, {5, 1}}},
      {"10100110101111", {{13, 5}}}};

  for (const auto& test : tests) {
    auto vec = bit_vector(test.sequence);
    auto bm = bitmap(vec);
    for (const auto& exp : test.expectations) {
      CHECK(bm.rank_0(exp.pos) == exp.count);
    }
  }
}

TEST_CASE("bitmap::select_1()") {
  struct expectation {
    index_type pos;
    size_type count;
  };

  struct test_case {
    std::string sequence;
    std::vector<expectation> expectations;
  };

  std::vector<test_case> tests = {
      {"10100110101111", {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {5, 5}}},
      {"10100110101110", {{0, 0}, {1, 1}, {2, 2}, {3, 3}, {5, 4}}},
      {"10100110101111010011010111101001101011110100110101111010011010111101001"
       "00110101110",
       {{0, 0}, {11, 7}, {64, 39}, {37, 23}}},
      {"10100110101111010011010111101001101011110100111101001101011110100",
       {{0, 0}, {64, 39}, {20, 13}, {38, 23}, {56, 35}}},
      {"10100110101111010011010111101001101011110100111101001101011110101010011"
       "01011110100110101111010011010111101001111010011010111101010100110101111"
       "01001101011110100110101111010011110100110101111010101001101011110100110"
       "10111101001101011110100111101001101011110101010011010111101001101011110"
       "10011010111101001111010011010111101000000101001101011110100110101111010"
       "01101011110100111101001101011110100",
       {{389, 234}, {381, 230}, {250, 150}, {30, 19}}},
      {"11010011010111101001101011110100110101111010011110100110101111010101010"
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
       {{1110, 670},
        {1326, 800},
        {926, 560},
        {1159, 700},
        {1161, 701},
        {1163, 702},
        {1164, 703},
        {1167, 704},
        {1331, 804},
        {1408, 850},
        {-1, 860},
        {-1, 900}}}};

  for (const auto& test : tests) {
    auto vec = bit_vector(test.sequence);
    auto bm = bitmap(vec);
    for (const auto& exp : test.expectations) {
      CHECK(bm.select_1(exp.count) == exp.pos);
    }
  }
}

TEST_CASE("bitmap::select_0()") {
  struct expectation {
    index_type pos;
    size_type count;
  };

  struct test_case {
    std::string sequence;
    std::vector<expectation> expectations;
  };

  std::vector<test_case> tests = {
      {"10100110101111", {{9, 3}, {6, 2}, {4, 1}}},
      {"11010011010111101001101011110100110101111010011110100110101111010101010"
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
       {{1081, 430},
        {1083, 431},
        {1255, 499},
        {1257, 500},
        {1267, 503},
        {1268, 504},
        {1411, 560},
        {-1, 600},
        {-1, 700}}}};

  for (const auto& test : tests) {
    auto vec = bit_vector(test.sequence);
    auto bm = bitmap(vec);
    for (const auto& exp : test.expectations) {
      CHECK(bm.select_0(exp.count) == exp.pos);
    }
  }
}
