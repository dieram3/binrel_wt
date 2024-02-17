#ifndef BRWT_CONCEPTS_H
#define BRWT_CONCEPTS_H

#include "brwt/type_traits.h"

namespace brwt {

template <typename T>
concept large_unsigned_integer = is_large_unsigned_integer<T>::value;

} // namespace brwt

#endif
