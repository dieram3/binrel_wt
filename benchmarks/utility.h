#ifndef BINREL_WT_BENCHMARKS_UTILITY_H // NOLINT
#define BINREL_WT_BENCHMARKS_UTILITY_H

#include <cassert>     // assert
#include <random>      // mt19937_64, random_device, uniform_int_distribution
#include <type_traits> // is_integral

namespace brwt {
namespace benchmark {

/// Returns two nth power of two.
constexpr int pow_2(const int nth) {
  return 1 << nth;
}

inline auto& get_random_engine() {
  static std::mt19937_64 engine{std::random_device{}()};
  return engine;
}

template <typename T>
T gen_integer(const T min, const T max) {
  static_assert(std::is_integral<T>::value, "");
  assert(min <= max);

  std::uniform_int_distribution<T> dist{min, max};
  return dist(get_random_engine());
}

} // end namespace benchmark
} // end namespace brwt

#endif // BINREL_WT_BENCHMARKS_UTILITY_H
