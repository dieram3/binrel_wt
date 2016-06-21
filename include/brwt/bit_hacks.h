//          Copyright DJP Team 2015
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BRWT_BIT_HACKS_H
#define BRWT_BIT_HACKS_H

#include <cassert>     // assert
#include <cstddef>     // size_t
#include <limits>      // numeric_limits
#include <type_traits> // is_unsigned

namespace brwt {

inline int pop_count(unsigned x) {
  return __builtin_popcount(x);
}
inline int pop_count(unsigned long x) {
  return __builtin_popcountl(x);
}
inline int pop_count(unsigned long long x) {
  return __builtin_popcountll(x);
}

// Returns a mask with the 'count' least significant bits set.
// Requires: count < bits_of(T)
//
template <typename T>
constexpr T lsb_mask(const std::size_t count) {
  static_assert(std::is_unsigned<T>::value, "");
  assert(count < std::numeric_limits<T>::digits);
  return (T{1} << count) - 1;
}

} // end namespace brwt

#endif // BRWT_BIT_HACKS_H
