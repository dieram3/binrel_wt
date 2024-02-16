export module brwt.concepts;

import brwt.type_traits;

export namespace brwt {

template <typename T>
concept large_unsigned_integer = is_large_unsigned_integer<T>::value;

} // namespace brwt
