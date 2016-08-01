#include <brwt/bit_vector.h>
#include <brwt/bitmap.h>

#include "utils/array_view.h" // array_view
#include <brwt/bit_hacks.h>   // pop_count, rank_0, rank_1, used_bits
#include <brwt/utility.h>     // ceil_div
#include <algorithm>          // min
#include <cassert>            // assert
#include <limits>             // numeric_limits
#include <type_traits>        // enable_if_t
#include <utility>            // move

namespace brwt {

using block_t = bit_vector::block_type;

// ===----------------------------------------------------===
//                     bit_hacks extensions
// ===----------------------------------------------------===

// TODO(Diego): Use int_binary_search.
//
template <typename T, typename Pred>
static constexpr T binary_search(T a, T b, Pred pred) {
  assert(a <= b);
  while (a != b) {
    const T mid = a + (b - a) / 2;
    if (pred(mid)) {
      a = mid + 1;
    } else {
      b = mid;
    }
  }
  return a;
}

template <typename T>
using enable_if_word = std::enable_if_t<is_word_type<T>>;

template <typename T, typename = enable_if_word<T>>
static constexpr int size_in_bits = std::numeric_limits<T>::digits;

template <bool B, typename T, typename = enable_if_word<T>>
static constexpr int count(const T value) {
  return B ? pop_count(value) : pop_count(~value);
}

template <bool B, typename T, typename = enable_if_word<T>>
static constexpr int rank(const T value, const int nth) {
  return B ? rank_1(value, nth) : rank_0(value, nth);
}

template <typename T, typename = enable_if_word<T>>
static constexpr int select_1(const T value, const int nth) {
  assert(nth > 0 && nth <= pop_count(value));
  auto not_enough = [value, nth](const int pos) {
    return rank_1(value, pos) < nth;
  };
  return binary_search(nth - 1, size_in_bits<T> - 1, not_enough);
}

template <typename T, typename = enable_if_word<T>>
static constexpr int select_0(const T value, const int nth) {
  return select_1(~value, nth);
}

template <bool B, typename T, typename = enable_if_word<T>>
static constexpr int select(const T value, const int nth) {
  return B ? select_1(value, nth) : select_0(value, nth);
}

// ===----------------------------------------------------===
//             Bitmap implementation
// ===----------------------------------------------------===

static constexpr size_type bits_per_block = bit_vector::bits_per_block;
static constexpr size_type blocks_per_super_block = 8;
static constexpr size_type bits_per_super_block =
    blocks_per_super_block * bits_per_block;

static_assert(is_power_of_two(bits_per_block), "");
static_assert(is_power_of_two(blocks_per_super_block), "");
static_assert(is_power_of_two(bits_per_super_block), "");

/// Returns the range of blocks belonging to the super block `sb_idx`.
///
auto bitmap::blocks_of_super_block(const size_type sb_idx) const noexcept {
  array_view<block_t> v{sequence.data(), sequence.num_blocks()};
  return v.subarray(/*pos=*/sb_idx * blocks_per_super_block,
                    /*count=*/blocks_per_super_block);
}

/// Sequentially counts the number of set bits in the given range of blocks.
///
static constexpr size_type pop_count(const array_view<block_t> blocks) {
  size_type sum = 0;
  for (const auto elem : blocks) {
    sum += pop_count(elem);
  }
  return sum;
}

bitmap::bitmap(bit_vector vec) : sequence(std::move(vec)) {

  super_blocks = [this] {
    const auto count = ceil_div(sequence.num_blocks(), blocks_per_super_block);
    const int bpe = used_bits(static_cast<word_type>(sequence.size()));
    return int_vector(count, bpe);
  }();

  size_type acc_sum = 0;
  for (size_type i = 0; i < super_blocks.size(); ++i) {
    // TODO(Diego): Try to use transform_inclusive_scan when available.
    acc_sum += pop_count(blocks_of_super_block(i));
    super_blocks[i] = static_cast<word_type>(acc_sum);
  }
}

template <bool B>
auto bitmap::num_of() const noexcept -> size_type {
  return B ? num_ones() : num_zeros();
}

// Rank lands ----------------------

template <>
auto bitmap::sb_rank<true>(const index_type sb_idx) const noexcept
    -> size_type {
  assert(sb_idx < super_blocks.size());

  return static_cast<size_type>(super_blocks[sb_idx]);
}

template <>
auto bitmap::sb_rank<false>(const index_type sb_idx) const noexcept
    -> size_type {
  assert(sb_idx < super_blocks.size());

  const auto total = std::min((sb_idx + 1) * bits_per_super_block, size());
  return total - sb_rank<true>(sb_idx);
}

template <bool B>
auto bitmap::sb_exclusive_rank(const size_type sb_idx) const noexcept
    -> size_type {
  assert(sb_idx <= super_blocks.size());
  return sb_idx == 0 ? 0 : sb_rank<B>(sb_idx - 1);
}

auto bitmap::rank_1(const index_type pos) const noexcept -> size_type {
  assert(pos >= 0 && pos < length());

  // Address
  const auto num_super_block = pos / bits_per_super_block;
  const auto num_block = pos / bits_per_block;
  const int num_bit = pos % bits_per_block;

  size_type sum = sb_exclusive_rank<1>(num_super_block);

  for (index_type ith = (num_super_block * blocks_per_super_block);
       ith < num_block; ++ith) {
    sum += pop_count(sequence.get_block(ith));
  }

  sum += brwt::rank_1(sequence.get_block(num_block), num_bit);

  return sum;
}

auto bitmap::rank_0(const index_type pos) const noexcept -> size_type {
  return (pos + 1) - rank_1(pos);
}

// Select lands ----------------------

template <bool B>
auto bitmap::sb_select(const size_type nth) const noexcept -> size_type {
  assert(nth > 0);
  assert(nth <= num_of<B>());
  assert(!super_blocks.empty());

  auto not_enough = [&](const index_type pos) { return sb_rank<B>(pos) < nth; };
  const auto sb_begin = (nth - 1) / bits_per_super_block;

  return binary_search(sb_begin, super_blocks.size() - 1, not_enough);
}

/// Sequentially searches for the nth bit set to B.
///
/// \pre The nth bit must exist in the range of blocks.
///
/// \returns The position of the found bit, measured in bits. That is, the
/// position of the bit in the containing block plus `size_in_bits<T>`
/// multiplied by the number of preceding blocks.
///
template <bool B, typename T, typename = enable_if_word<T>>
static constexpr index_type sequential_select(const array_view<T> blocks,
                                              const size_type nth) {
  assert(nth > 0);

  size_type sum = 0;
  for (index_type i = 0; i < blocks.size(); ++i) {
    const auto& elem = blocks[i];

    const auto prev_sum = sum;
    sum += count<B>(elem);

    if (sum >= nth) {
      assert(nth > prev_sum);
      return i * size_in_bits<T> +
             select<B>(elem, static_cast<int>(nth - prev_sum));
    }
  }
  return index_npos;
}

template <bool B>
auto bitmap::select(size_type nth) const noexcept -> index_type {
  assert(nth > 0);

  if (nth > num_of<B>()) {
    return index_npos; // The answer does not exist.
  }

  const auto sb_idx = sb_select<B>(nth);
  assert(sb_idx < super_blocks.size());

  nth -= sb_exclusive_rank<B>(sb_idx);
  assert(nth > 0 && nth <= bits_per_super_block);

  return sb_idx * bits_per_super_block +
         sequential_select<B>(blocks_of_super_block(sb_idx), nth);
}

auto bitmap::select_1(size_type nth) const noexcept -> index_type {
  assert(nth > 0);
  return select<1>(nth);
}

auto bitmap::select_0(size_type nth) const noexcept -> index_type {
  assert(nth > 0);
  return select<0>(nth);
}

} // end namespace brwt
