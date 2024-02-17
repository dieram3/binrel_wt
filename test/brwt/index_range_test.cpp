#include "brwt/index_range.h"
#include <doctest/doctest.h>

namespace brwt::test {
namespace {

// TEST_SUITE("index_range");

TEST_CASE("index_range::index_range()") {
  const index_range range;

  CHECK(range.begin() == 0);
  CHECK(range.end() == 0);
}

// TODO(Diego): Test the other stuff.

} // namespace
} // namespace brwt::test
