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
  bitmap() = default;
  explicit bitmap(bit_vector vec);

  bool access(index_type pos) const;

  size_type rank_0(index_type pos) const;
  size_type rank_1(index_type pos) const;

  index_type select_0(size_type nth) const;
  index_type select_1(size_type nth) const;

  size_type length() const;
  size_type size() const;

  size_type num_ones() const;
  size_type num_zeros() const;

private:
  bit_vector sequence;
  int_vector super_blocks;
};

// ==========================================
// Inline definitions
// ==========================================

inline bool bitmap::access(const index_type pos) const {
  return sequence.get(pos);
}

inline bitmap::size_type bitmap::rank_0(const index_type pos) const {
  return (pos + 1) - rank_1(pos);
}

inline bitmap::size_type bitmap::length() const {
  return sequence.length();
}

inline bitmap::size_type bitmap::size() const {
  return sequence.length();
}

inline bitmap::size_type bitmap::num_ones() const {
  if (size() == 0) {
    return 0;
  }
  assert(super_blocks.size() > 0);
  return static_cast<size_type>(super_blocks[super_blocks.size() - 1]);
}

inline bitmap::size_type bitmap::num_zeros() const {
  return size() - num_ones();
}

} // end namespace brwt

#endif // BRWT_BITMAP_H
