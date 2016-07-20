#ifndef BRWT_COMMON_TYPES_H
#define BRWT_COMMON_TYPES_H

#include <cstddef> // ptrdiff_t
#include <cstdint> // uint_fast64_t

namespace brwt {
namespace types {
// TODO(Diego): Deprecate the use of 'types' namespace, remove the use of it
// along the library, then remove it. Types will be in brwt namespace now.

using size_type = std::ptrdiff_t;
using index_type = size_type;
using word_type = std::uint_fast64_t;

} // end namespace types

// generic types

using types::size_type;
using types::index_type;
using types::word_type;

// wavelet tree types

using symbol_id = word_type;

// condition types (used as function parameters)

template <typename T>
struct less_equal {
  T max_value;
};

template <typename T>
struct greater_equal {
  T min_value;
};

template <typename T>
struct between {
  T min_value;
  T max_value;
};

} // end namespace brwt

#endif // BRWT_COMMON_TYPES_H
