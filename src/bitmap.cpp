#include <brwt/bitmap.h>
#include <brwt/int_vector.h>

#include <brwt/bit_hacks.h> // pop_count
#include <brwt/utility.h>   // ceil_div
#include <cassert>          // assert
#include <cmath>            // log2, ceil
#include <utility>          // move

using brwt::bitmap;
using brwt::int_vector;

using value_type = int_vector::value_type;
using size_type = bitmap::size_type;

// super_blocks[i] = rank_1(num_elems_per_super_blocks * (i+1) - 1);
static constexpr size_type bits_per_super_block = 640;
static constexpr size_type bits_per_block = 64;

bitmap::bitmap(bit_vector vec) : sequence(vec) {
  const size_type n = vec.length();

  const size_type num_super_blocks = ceil_div(n, bits_per_super_block) - 1;
  {
    const int bits = static_cast<int>(std::ceil(std::log2(n)));
    super_blocks = int_vector(num_super_blocks, bits);
  }

  size_type sum = 0;

  for (size_type i = 0; i < num_super_blocks; ++i) {
    const auto base = i * bits_per_super_block;
    for (size_type j = 0; j != bits_per_super_block; j += bits_per_block) {
      const auto pos = base + j;
      sum += pop_count(sequence.get_block(pos / bits_per_block));
    }
    super_blocks[i] = static_cast<value_type>(sum);
  }
}

bitmap::index_type bitmap::select_1(const size_type nth) const {
  assert(nth < length());

  // lower bound in super blocks
  index_type idx = 0;
  size_type count = 0;
  {
    index_type last = super_blocks.length();
    index_type first = 0;
    index_type sb_idx = -1;

    while (last > first) {
      auto pos = first + (last - first) / 2;
      auto bits = static_cast<size_type>(super_blocks[pos]);

      if (bits < nth) {
        first = pos + 1;
        sb_idx = pos;
      } else {
        last = pos;
      }
    }

    if (sb_idx >= 0) {
      idx = (sb_idx + 1) * bits_per_super_block;
      count = static_cast<size_type>(super_blocks[sb_idx]);
    }
  }

  // sequential search in blocks (at most 10 popcounts)
  {
    auto last = std::min(idx + bits_per_super_block, length());
    for (; idx + bits_per_block < last; idx += bits_per_block) {
      auto bits = pop_count(sequence.get_block(idx / bits_per_block));
      if (count + bits > nth) {
        break;
      }
      count += bits;
    }
  }

  // last binary search
  {
    auto last = std::min(length() - idx, size_type{bits_per_block});
    auto first = size_type{0};

    auto idx_diff = index_type{0};

    while (last > first) {
      auto pos = first + (last - first) / 2;
      auto bits = pop_count(sequence.get_chunk(idx, pos));
      if (count + bits < nth) {
        first = pos + 1;
        idx_diff = static_cast<index_type>(pos);
      } else {
        last = pos;
      }
    }
    idx += idx_diff;
  }

  return idx;
}

bitmap::index_type bitmap::select_0(const index_type nth) const {
  assert(nth < length());

  // This needs to be improved, just a binary search
  auto last = length() - 1;
  auto first = size_type{0};
  auto idx = index_type{0};

  while (first < last) {
    auto pos = first + (last - first) / 2;
    auto count = rank_0(pos);

    if (count < nth) {
      first = pos + 1;
      idx = pos;
    } else {
      last = pos;
    }
  }

  // final sequential search
  for (; rank_0(idx) != nth && idx < length(); ++idx) {
  }

  return idx;
}

bitmap::size_type bitmap::rank_1(const index_type pos) const {
  assert(pos < length());

  size_type sum = [&] {
    const index_type idx = (pos / bits_per_super_block) - 1;
    return (idx >= 0) ? static_cast<size_type>(super_blocks[idx]) : 0;
  }();

  size_type current_pos = pos - (pos % bits_per_super_block);
  for (; current_pos + bits_per_block <= pos; current_pos += bits_per_block) {
    sum += pop_count(sequence.get_block(current_pos / bits_per_block));
  }

  if (current_pos <= pos) {
    sum += pop_count(sequence.get_chunk(current_pos, (pos - current_pos) + 1));
  }

  return sum;
}
