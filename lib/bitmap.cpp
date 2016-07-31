#include <brwt/bit_vector.h>
#include <brwt/bitmap.h>

#include <brwt/bit_hacks.h> // pop_count, rank_0, rank_1, used_bits
#include <brwt/utility.h>   // ceil_div

#include <algorithm> // min
#include <cassert>   // assert
#include <utility>   // move, pair

namespace brwt {

using value_type = bit_vector::block_type;

// TODO(Diego): Rewrite some code. A bit of pessimization was made to fix the
// errors quickly.

// ==========================================
// helper functions
// ==========================================

template <typename T, typename Pred>
static T binary_search(T a, T b, Pred pred) {
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

static constexpr index_type operator"" _idx(unsigned long long pos) {
  return static_cast<index_type>(pos);
}

// ==========================================
// bitmap implementation
// ==========================================

static constexpr size_type bits_per_block = bit_vector::bits_per_block;
static constexpr size_type blocks_per_super_block = 8;
static constexpr size_type bits_per_super_block =
    blocks_per_super_block * bits_per_block;

static_assert(is_power_of_two(bits_per_block), "");
static_assert(is_power_of_two(blocks_per_super_block), "");
static_assert(is_power_of_two(bits_per_super_block), "");

bitmap::bitmap(bit_vector vec) : sequence(std::move(vec)) {

  const auto step = blocks_per_super_block; // measured in blocks.
  super_blocks = [this] {
    // num super-blocks
    const size_type count = ceil_div(sequence.num_blocks(), step);
    // bits per element
    const int bpe = used_bits(static_cast<word_type>(sequence.size()));
    return int_vector(count, bpe);
  }();

  auto sum_blocks = [&](index_type first, const index_type last) {
    size_type res = 0;
    for (; first != last; ++first) {
      res += pop_count(sequence.get_block(first));
    }
    return res;
  };

  size_type acc_sum = 0;
  for (size_type i = 0; i < super_blocks.size(); ++i) {
    const auto first = i * step;
    const auto last = std::min(first + step, sequence.num_blocks());
    acc_sum += sum_blocks(first, last);
    super_blocks[i] = static_cast<word_type>(acc_sum);
  }
}

template <typename Rank>
static std::pair<index_type, size_type>
rank_find(const bit_vector& vec, const size_type start, const size_type value,
          const Rank rank) {
  size_type sum = 0;
  for (size_type i = start; i < vec.num_blocks(); ++i) {
    const auto tpm = sum + rank(vec.get_block(i));
    if (tpm >= value) {
      return std::make_pair(i, sum);
    }
    sum = tpm;
  }
  return std::make_pair(vec.num_blocks(), sum);
}

template <typename T>
static int select_1(const T value, const int nth) {
  static_assert(std::is_unsigned<T>::value, "");
  assert(nth > 0 && nth <= rank_1(value));
  auto not_enough = [value, nth](const int pos) {
    return rank_1(value, pos) < nth;
  };
  return binary_search(nth - 1, std::numeric_limits<T>::digits - 1, not_enough);
}

template <typename T>
static int select_0(const T value, const int nth) {
  static_assert(is_word_type<T>, "");
  return select_1(~value, nth);
}

bitmap::index_type bitmap::select_1(size_type nth) const {
  assert(nth > 0);

  if (nth > num_ones()) {
    return index_npos; // The answer does not exist.
  }

  const auto sb_idx = [&] {
    auto ones_sb = [this](const index_type pos) {
      return static_cast<size_type>(super_blocks[pos]);
    };
    auto not_enough = [&](const index_type pos) { return ones_sb(pos) < nth; };
    const auto pos = binary_search(0_idx, super_blocks.size(), not_enough);
    nth -= (pos == 0 ? 0 : ones_sb(pos - 1));
    return pos;
  }();
  assert(sb_idx < super_blocks.size());
  assert(nth > 0 && nth <= bits_per_super_block);

  const auto seq_idx = [&] {
    const auto start = sb_idx * blocks_per_super_block;
    const auto pair = rank_find(sequence, start, nth, [](const auto value) {
      return brwt::rank_1(value);
    });
    nth -= pair.second; // exclusive_count
    return pair.first;  // idx where the accumulated pop count was >= nth
  }();
  assert(nth > 0 && nth <= std::numeric_limits<word_type>::digits);

  return seq_idx * bits_per_block +
         brwt::select_1(sequence.get_block(seq_idx), static_cast<int>(nth));
}

bitmap::index_type bitmap::select_0(size_type nth) const {
  assert(nth > 0);

  if (nth > num_zeros()) {
    return index_npos; // The answer does not exist.
  }

  const auto sb_idx = [&] {
    // TODO(Diego): It seems that std::min can be removed safely.
    auto zeros_sb = [this](const index_type pos) {
      return std::min((pos + 1) * bits_per_super_block, size()) -
             static_cast<size_type>(super_blocks[pos]);
    };
    auto not_enough = [&](const index_type pos) { return zeros_sb(pos) < nth; };
    const auto pos = binary_search(0_idx, super_blocks.size(), not_enough);
    nth -= (pos == 0 ? 0 : zeros_sb(pos - 1));
    return pos;
  }();
  assert(sb_idx < super_blocks.size());
  assert(nth > 0 && nth <= bits_per_super_block);

  const auto seq_idx = [&] {
    const auto start = sb_idx * blocks_per_super_block;
    const auto pair = rank_find(sequence, start, nth, [](const auto elem) {
      return brwt::rank_0(elem);
    });
    nth -= pair.second; // exclusive_count
    return pair.first;  // idx where the accumulated pop count was >= nth
  }();
  assert(seq_idx < sequence.num_blocks());
  assert(nth > 0 && nth <= std::numeric_limits<word_type>::digits);

  return seq_idx * bits_per_block +
         brwt::select_0(sequence.get_block(seq_idx), static_cast<int>(nth));
}

static size_type sb_exclusive_rank(const int_vector& sb_rank,
                                   const index_type sb_pos) {
  assert(sb_pos >= 0 && sb_pos < sb_rank.size());
  if (sb_pos == 0) {
    return 0;
  }
  return static_cast<size_type>(sb_rank[sb_pos - 1]);
}

bitmap::size_type bitmap::rank_1(const index_type pos) const {
  assert(pos >= 0 && pos < length());

  // Address
  const auto num_super_block = pos / bits_per_super_block;
  const auto num_block = pos / bits_per_block;
  const int num_bit = pos % bits_per_block;

  size_type sum = sb_exclusive_rank(super_blocks, num_super_block);

  for (index_type ith = (num_super_block * blocks_per_super_block);
       ith < num_block; ++ith) {
    sum += pop_count(sequence.get_block(ith));
  }

  sum += brwt::rank_1(sequence.get_block(num_block), num_bit);

  return sum;
}

} // end namespace brwt
