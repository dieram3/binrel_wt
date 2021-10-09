#ifndef BRWT_BIT_VECTOR_H
#define BRWT_BIT_VECTOR_H

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <vector>

namespace brwt {

class bit_vector {
public:
  using size_type = std::ptrdiff_t;
  using block_type = std::uint_fast64_t;

  static constexpr size_type bits_per_block =
      std::numeric_limits<block_type>::digits;

  bit_vector() = default;
  explicit bit_vector(size_type count);
  explicit bit_vector(size_type count, block_type value);
  explicit bit_vector(const std::string& s);

  size_type length() const noexcept;
  size_type size() const noexcept;
  size_type num_blocks() const noexcept;
  size_type allocated_bytes() const noexcept;

  bool get(size_type pos) const noexcept;
  void set(size_type pos, bool value) noexcept;

  block_type get_chunk(size_type pos, size_type count) const noexcept;
  void set_chunk(size_type pos, size_type count, block_type value) noexcept;

  block_type get_block(size_type num_block) const noexcept;
  void set_block(size_type num_block, block_type value) noexcept;

  /// \brief Returns a span to the underlying array of blocks.
  ///
  /// Note that the unused bits from the last block are set to zero.
  ///
  std::span<const block_type> get_blocks() const noexcept {
    return m_blocks;
  }

private:
  size_type m_len{};
  std::vector<block_type> m_blocks;
};

// ==========================================
// Inline definitions
// ==========================================

inline auto bit_vector::length() const noexcept -> size_type {
  return m_len;
}

inline auto bit_vector::size() const noexcept -> size_type {
  return m_len;
}

inline auto bit_vector::num_blocks() const noexcept -> size_type {
  return static_cast<size_type>(m_blocks.size());
}

inline auto bit_vector::get_block(const size_type num_block) const noexcept
    -> block_type {
  return m_blocks[num_block];
}

inline void bit_vector::set_block(const size_type num_block,
                                  const block_type value) noexcept {
  m_blocks[num_block] = value;
}

} // namespace brwt

#endif // BRWT_BIT_VECTOR_H
