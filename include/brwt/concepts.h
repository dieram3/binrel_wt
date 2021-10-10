#ifndef BRWT_CONCEPTS_H
#define BRWT_CONCEPTS_H

#include "brwt/type_traits.h"
#include <type_traits>

namespace brwt {

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept large_unsigned_integer = is_large_unsigned_integer<T>::value;

} // namespace brwt

#endif
