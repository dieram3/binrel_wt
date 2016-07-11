#ifndef BRWT_COMMON_TYPES_H
#define BRWT_COMMON_TYPES_H

#include <cstddef> // ptrdiff_t
#include <cstdint> // uint_fast64_t

namespace brwt {
namespace types {

using size_type = std::ptrdiff_t;
using index_type = size_type;
using word_type = std::uint_fast64_t;

} // end namespace types
} // end namespace brwt

#endif // BRWT_COMMON_TYPES_H
