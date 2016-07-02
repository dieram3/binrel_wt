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
  static_assert(std::is_unsigned<T>::value, "The mask type must be unsigned");
  assert(count < std::numeric_limits<T>::digits);
  return (T{1} << count) - 1;
}

} // end namespace brwt

#endif // BRWT_BIT_HACKS_H
