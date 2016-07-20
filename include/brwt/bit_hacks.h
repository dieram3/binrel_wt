#ifndef BRWT_BIT_HACKS_H
#define BRWT_BIT_HACKS_H

#include <cassert>     // assert
#include <limits>      // numeric_limits
#include <type_traits> // is_unsigned

namespace brwt {

constexpr int pop_count(unsigned x) noexcept {
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
template <typename T>
constexpr T lsb_mask(const int count) noexcept {
  // TODO(Diego): Consider rename this function to low_mask
  static_assert(std::is_unsigned<T>::value, "The mask type must be unsigned");
  assert(count < std::numeric_limits<T>::digits);
  return (T{1} << count) - 1;
}

/// \brief Returns the number of used bits in x.
///
/// Effectively returns 1 plus the position of the most significant bit.
///
/// \pre <tt>x != 0</tt>
///
template <typename T>
constexpr int used_bits(const T x) {
  static_assert(std::is_unsigned<T>::value, "");
  assert(x != 0);
  return std::numeric_limits<T>::digits - count_leading_zeros(x);
}

} // end namespace brwt

#endif // BRWT_BIT_HACKS_H
