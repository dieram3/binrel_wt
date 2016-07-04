#ifndef BRWT_BITMAP_H
#define BRWT_BITMAP_H

#include <brwt/bit_vector.h> // bit_vector
#include <brwt/int_vector.h> // int_vector
#include <cstddef>           // ptrdiff_t
#include <vector>            // vector

namespace brwt {

class bitmap {
public:
  using index_type = std::ptrdiff_t;
  using size_type = index_type;

public:
  bitmap() = default;
  explicit bitmap(bit_vector vec);

  bool access(index_type pos) const;

  size_type rank_0(index_type pos) const;
  size_type rank_1(index_type pos) const;

  index_type select_0(size_type nth) const;
  index_type select_1(size_type nth) const;

  size_type length() const;

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

} // end namespace brwt

#endif // BRWT_BITMAP_H
