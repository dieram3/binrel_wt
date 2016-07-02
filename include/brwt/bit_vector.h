#ifndef BRWT_BIT_VECTOR_H
#define BRWT_BIT_VECTOR_H

#include <cstddef> // ptrdiff_t
#include <cstdint> // uint_fast64_t
#include <string>  // string
#include <vector>  // vector

namespace brwt {

class bit_vector {
public:
  using size_type = std::ptrdiff_t;
  using block_type = std::uint_fast64_t;

public:
  bit_vector() = default;
  explicit bit_vector(size_type count);
  explicit bit_vector(size_type count, block_type value);
  explicit bit_vector(const std::string& s);

  size_type length() const noexcept;
  size_type allocated_bytes() const noexcept;

  bool get(size_type pos) const noexcept;
  void set(size_type pos, bool value) noexcept;

  block_type get_chunk(size_type pos, size_type count) const noexcept;
  void set_chunk(size_type pos, size_type count, block_type value) noexcept;

private:
  size_type m_len{};
  std::vector<block_type> blocks;
};

// ==========================================
// Inline definitions
// ==========================================

inline bit_vector::size_type bit_vector::length() const noexcept {
  return m_len;
}

} // end namespace brwt

#endif // BRWT_BIT_VECTOR_H
