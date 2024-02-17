#ifndef BRWT_UTILITY_H
#define BRWT_UTILITY_H

#include "brwt/concepts.h"
#include <cassert>

namespace brwt {

/// \brief Computes the ceil of a real division between two integers.
///
/// \param a The dividend.
/// \param b The divisor.
///
/// \returns The ceil of <tt>a / b</tt>.
///
/// \pre <tt>b != 0</tt>
///
template <integral T>
constexpr T ceil_div(T a, T b) noexcept {
  assert(b != 0 && "cannot divide by zero");
  return a / b + T(a % b == 0 ? 0 : 1);
}

} // namespace brwt

#endif // BRWT_UTILITY_H
