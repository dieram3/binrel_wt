//          Copyright DJP Team 2015
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BRWT_UTILITY_H
#define BRWT_UTILITY_H

namespace brwt {

// Computes the ceil of <tt>a / b</tt>
//
template <typename T>
constexpr T ceil_div(T a, T b) {
  return a / b + T(a % b == 0 ? 0 : 1);
}

} // end namespace brwt

#endif // BRWT_UTILITY_H
