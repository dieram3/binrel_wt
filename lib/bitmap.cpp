#include <brwt/bit_vector.h>
#include <brwt/bitmap.h>

#include <brwt/bit_hacks.h> // pop_count
#include <brwt/utility.h>   // ceil_div

#include <algorithm> // min
#include <cassert>   // assert
#include <cmath>     // log2, ceil
#include <utility>   // move

using brwt::bitmap;
using brwt::bit_vector;

using value_type = bit_vector::block_type;
using size_type = bitmap::size_type;

// ==========================================
// helper functions
// ==========================================

template <typename T, typename Pred>
static T binary_search(T a, T b, Pred pred) {
  while (a != b) {
    const T mid = a + (b - a) / 2;
    if (pred(mid))
      a = mid + 1;
    else
      b = mid;
  }
  return a;
}

static constexpr value_type make_mask(const size_type count) noexcept {
  return brwt::lsb_mask<value_type>(static_cast<int>(count));
}

// ==========================================
// bitmap implementation
// ==========================================

static constexpr size_type bits_per_block = bit_vector::bits_per_block;
static constexpr size_type blocks_per_super_block = 10;
static constexpr size_type bits_per_super_block =
    blocks_per_super_block * bits_per_block;

bitmap::bitmap(bit_vector vec) : sequence(std::move(vec)) {
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
  assert(nth > 0);

  const auto num_blocks = length() / bits_per_block;
  const auto num_super_blocks = super_blocks.length();

  // lower bound in super blocks
  index_type idx = 0;
  size_type count = 0;
  {
    index_type first = 0;
    index_type last = num_super_blocks;

    auto valid_block = [&](index_type current_idx) {
      const auto ones_count = static_cast<size_type>(super_blocks[current_idx]);
      return ones_count < nth;
    };

    auto super_block_idx = binary_search(first, last, valid_block);

    if (super_block_idx > 0 && valid_block(super_block_idx - 1)) {
      idx = super_block_idx * bits_per_super_block;
      count = static_cast<size_type>(super_blocks[super_block_idx - 1]);
    }
  }

  // sequential search in blocks (at most 10 popcounts)
  {
    auto first = idx / bits_per_block;
    auto last = std::min(first + blocks_per_super_block, num_blocks);

    for (; first != last; ++first) {
      auto ones_count = pop_count(sequence.get_block(first));
      if (count + ones_count > nth) {
        break;
      }
      count += ones_count;
    }

    idx = first * bits_per_block;

    if (count < nth) {
      auto next_block = sequence.get_block(first);

      auto valid_bits_count = [&](const size_type bits) {
        auto ones_count = pop_count(next_block & make_mask(bits));
        return count + ones_count < nth;
      };

      auto max_bits = std::min(bits_per_block, length() - idx);
      auto num_bits = binary_search(size_type{0}, max_bits, valid_bits_count);

      assert(num_bits > 0);

      idx += (num_bits - 1);
      count += pop_count(next_block & make_mask(num_bits));
    }
  }

  return (count == nth) ? idx : -1;
}

bitmap::index_type bitmap::select_0(const index_type nth) const {
  assert(nth > 0);

  const auto num_blocks = length() / bits_per_block;
  const auto num_super_blocks = super_blocks.length();

  index_type idx = 0;
  size_type count = 0;
  {
    index_type first = 0;
    index_type last = num_super_blocks;

    auto valid_block = [&](index_type current_idx) {
      const auto zeros_count =
          bits_per_super_block * (current_idx + 1) -
          static_cast<size_type>(super_blocks[current_idx]);
      return zeros_count < nth;
    };

    auto super_block_idx = binary_search(first, last, valid_block);

    if (super_block_idx > 0 && valid_block(super_block_idx - 1)) {
      idx = super_block_idx * bits_per_super_block;
      count = bits_per_super_block * (super_block_idx) -
              static_cast<size_type>(super_blocks[super_block_idx - 1]);
    }
  }

  // sequential search in blocks (at most 10 popcounts)
  {
    auto first = idx / bits_per_block;
    auto last = std::min(first + blocks_per_super_block, num_blocks);

    for (; first < last; ++first) {
      auto zeros_count = bits_per_block - pop_count(sequence.get_block(first));
      if (count + zeros_count > nth) {
        break;
      }
      count += zeros_count;
    }

    idx = first * bits_per_block;

    if (count < nth) {
      auto next_block = sequence.get_block(first);

      auto valid_bits_count = [&](const size_type bits) {
        auto zeros_count = bits - pop_count(next_block & make_mask(bits));
        return count + zeros_count < nth;
      };

      auto max_bits = std::min(bits_per_block, length() - idx);
      auto num_bits = binary_search(size_type{0}, max_bits, valid_bits_count);
      assert(num_bits > 0);

      idx += (num_bits - 1);
      count += num_bits - pop_count(next_block & make_mask(num_bits));
    }
  }

  return (count == nth) ? idx : -1;
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
