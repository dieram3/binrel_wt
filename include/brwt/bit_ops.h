#ifndef BRWT_BIT_OPS_H
#define BRWT_BIT_OPS_H

#include "brwt/concepts.h"
#include <bit>
#include <cassert>
#include <limits>
#include <type_traits>

namespace brwt {

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
  return std::numeric_limits<T>::digits - std::countl_zero(x);
}

template <large_unsigned_integer T>
constexpr int rank_1(const T value) noexcept {
  return std::popcount(value);
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

#endif // BRWT_BIT_OPS_H
