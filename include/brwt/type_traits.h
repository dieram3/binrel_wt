#ifndef BRWT_TYPE_TRAITS_H
#define BRWT_TYPE_TRAITS_H

#include <type_traits>

namespace brwt {

template <typename T, typename... Types>
using is_any_of = std::disjunction<std::is_same<T, Types>...>;

template <typename T>
using is_large_unsigned_integer = is_any_of<std::remove_cv_t<T>, unsigned int,
                                            unsigned long, unsigned long long>;

} // namespace brwt

#endif
