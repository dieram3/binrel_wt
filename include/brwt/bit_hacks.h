#ifndef BRWT_BIT_HACKS_H
#define BRWT_BIT_HACKS_H

#include "brwt/concepts.h"
#include <cassert>
#include <limits>
#include <type_traits>

namespace brwt {

constexpr int pop_count(unsigned x) noexcept {
  // TODO(Diego): Consider using constexpr-if when clang-format gets it right.
  return __builtin_popcount(x);
}
constexpr int pop_count(unsigned long x) noexcept {
  return __builtin_popcountl(x);
}
constexpr int pop_count(unsigned long long x) noexcept {
  return __builtin_popcountll(x);
}

constexpr int count_leading_zeros(unsigned x) noexcept {
  assert(x != 0);
  return __builtin_clz(x);
}
constexpr int count_leading_zeros(unsigned long x) noexcept {
  assert(x != 0);
  return __builtin_clzl(x);
}
constexpr int count_leading_zeros(unsigned long long x) noexcept {
  assert(x != 0);
  return __builtin_clzll(x);
}

constexpr int count_trailing_zeros(unsigned x) noexcept {
  assert(x != 0);
  return __builtin_ctz(x);
}
constexpr int count_trailing_zeros(unsigned long x) noexcept {
  assert(x != 0);
  return __builtin_ctzl(x);
}
constexpr int count_trailing_zeros(unsigned long long x) noexcept {
  assert(x != 0);
  return __builtin_ctzll(x);
}

/// \brief Creates a mask with the 'count' least significant bits set.
///
/// \param count The number of bits to be set in the mask.
///
/// \returns The created mask.
///
/// \pre <tt>count < std::numeric_limits<T>::digits</tt>
///
template <large_unsigned_integer T>
constexpr T lsb_mask(const int count) noexcept {
  // TODO(Diego): Consider rename this function to low_mask
  assert(count < std::numeric_limits<T>::digits);
  return (T{1} << count) - 1;
}

/// \brief Returns the number of used bits in x.
///
/// Effectively returns 1 plus the position of the most significant bit.
///
/// \pre <tt>x != 0</tt>
///
template <large_unsigned_integer T>
constexpr int used_bits(const T x) {
  assert(x != 0);
  return std::numeric_limits<T>::digits - count_leading_zeros(x);
}

template <large_unsigned_integer T>
constexpr int rank_1(const T value) noexcept {
  return pop_count(value);
}

template <large_unsigned_integer T>
constexpr int rank_0(const T value) noexcept {
  return rank_1(~value);
}

template <large_unsigned_integer T>
constexpr int rank_1(const T value, const int pos) noexcept {
  assert(pos >= 0 && pos < std::numeric_limits<T>::digits);

  return (pos + 1 == std::numeric_limits<T>::digits)
             ? rank_1(value)
             : rank_1(value & lsb_mask<T>(pos + 1));
}

template <large_unsigned_integer T>
constexpr int rank_0(const T value, const int pos) noexcept {
  assert(pos >= 0 && pos < std::numeric_limits<T>::digits);

  return rank_1(~value, pos);
}

/// \brief Checks if the input integer is a power of two.
///
/// \pre <tt>value > 0</tt>
///
template <integral T>
constexpr bool is_power_of_two(const T value) {
  assert(value > 0);

  return (value & (value - 1)) == 0;
}

} // namespace brwt

#endif // BRWT_BIT_HACKS_H
