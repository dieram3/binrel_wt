#ifndef BRWT_BITMAP_H
#define BRWT_BITMAP_H

#include <brwt/bit_vector.h>   // bit_vector
#include <brwt/common_types.h> // index_type, size_type
#include <brwt/int_vector.h>   // int_vector
#include <cassert>             // assert

namespace brwt {

class bitmap {
public:
  using index_type = brwt::index_type;
  using size_type = brwt::size_type;

public:
  bitmap() noexcept = default;
  explicit bitmap(bit_vector vec);

  bool access(index_type pos) const noexcept;

  size_type rank_0(index_type pos) const noexcept;
  size_type rank_1(index_type pos) const noexcept;

  index_type select_0(size_type nth) const noexcept;
  index_type select_1(size_type nth) const noexcept;

  size_type length() const noexcept; // TODO(Diego): Remove this.
  size_type size() const noexcept;

  size_type num_ones() const noexcept;
  size_type num_zeros() const noexcept;

private:
  /// Returns an array_view to the blocks contained in the super block `sb_idx`.
  auto blocks_of_super_block(index_type sb_idx) const noexcept;

  template <bool B>
  size_type num_of() const noexcept;

  template <bool B>
  size_type sb_rank(index_type sb_idx) const noexcept;

  template <bool B>
  size_type sb_exclusive_rank(index_type sb_idx) const noexcept;

  /// Finds the super block that contains the nth bit equal to B.
  template <bool B>
  index_type sb_select(size_type nth) const noexcept;

  /// Templated version of select_1 and select_0.
  template <bool B>
  index_type select(size_type nth) const noexcept;

  // member data
  bit_vector sequence;
  int_vector super_blocks;
};

// ==========================================
// Inline definitions
// ==========================================

inline bool bitmap::access(const index_type pos) const noexcept {
  return sequence.get(pos);
}

inline bitmap::size_type bitmap::length() const noexcept {
  return sequence.length();
}

inline bitmap::size_type bitmap::size() const noexcept {
  return sequence.length();
}

inline bitmap::size_type bitmap::num_ones() const noexcept {
  if (size() == 0) {
    return 0;
  }
  assert(super_blocks.size() > 0);
  return static_cast<size_type>(super_blocks[super_blocks.size() - 1]);
}

inline bitmap::size_type bitmap::num_zeros() const noexcept {
  return size() - num_ones();
}

} // end namespace brwt

#endif // BRWT_BITMAP_H
