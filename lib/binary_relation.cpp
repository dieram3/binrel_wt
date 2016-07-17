#include <brwt/binary_relation.h>

#include <brwt/bit_hacks.h> // used_bits
#include <algorithm>        // max, for_each
#include <cassert>          // assert
#include <cstddef>          // size_t
#include <iterator>         // begin, end
#include <numeric>          // partial_sum
#include <tuple>            // tie
#include <type_traits>      // is_enum, underlying_type_t
#include <utility>          // pair, make_pair, move
#include <vector>           // vector

namespace brwt {

using std::size_t;
using types::size_type;
using types::index_type;

using node_proxy = wavelet_tree::node_proxy;
using symbol_id = wavelet_tree::symbol_id;

using object_id = binary_relation::object_id;
using label_id = binary_relation::label_id;
using pair_type = binary_relation::pair_type;

// ==========================================
// Generic helpers
// ==========================================

template <typename T>
static const T& get_left(const std::pair<T, T>& p) {
  return p.first;
}

template <typename T>
static const T& get_right(const std::pair<T, T>& p) {
  return p.second;
}

// TODO(Diego): Consider to put exclusive_scan in a private header. It is also
// used in the wavelet tree implementation.
template <typename InputIt, typename OutputIt, typename T>
static void exclusive_scan(InputIt first, const InputIt last, OutputIt d_first,
                           T init) {
  for (; first != last; ++first) {
    *d_first++ = std::exchange(init, init + (*first));
  }
}

template <typename InputRange, typename UnaryFunction>
static UnaryFunction for_each(const InputRange& range, UnaryFunction f) {
  return std::for_each(std::begin(range), std::end(range), f);
}

template <typename T>
constexpr auto to_underlying_type(const T value) noexcept {
  static_assert(std::is_enum<T>::value, "");
  return static_cast<std::underlying_type_t<T>>(value);
}

// ==========================================
// node_proxy extensions
// ==========================================

static size_type inclusive_rank_0(const node_proxy& node,
                                  const index_type pos) noexcept {
  return node.rank_0(pos);
}

static size_type inclusive_rank_1(const node_proxy& node,
                                  const index_type pos) noexcept {
  return node.rank_1(pos);
}

static size_type exclusive_rank_0(const node_proxy& node,
                                  const index_type pos) noexcept {
  assert(pos >= 0 && pos <= node.size());
  if (pos == 0) {
    return 0;
  }
  return node.rank_0(pos - 1);
}

static size_type exclusive_rank_1(const node_proxy& node,
                                  const index_type pos) noexcept {
  assert(pos >= 0 && pos <= node.size());
  if (pos == 0) {
    return 0;
  }
  return node.rank_1(pos - 1);
}

static size_type exclusive_rank(const wavelet_tree& wt, const symbol_id symbol,
                                const index_type pos) noexcept {
  assert(pos >= 0 && pos <= wt.size());
  if (pos == 0) {
    return 0;
  }
  return wt.rank(symbol, pos);
}

// ==========================================
// class index_range
// ==========================================

namespace {
class index_range {
public:
  index_range(index_type first, index_type last) noexcept
      : m_first{first}, m_size{last - first} {
    assert(begin() >= 0);
    assert(size() >= 0);
  }

  index_type begin() const noexcept {
    return m_first;
  }
  size_type size() const noexcept {
    return m_size;
  }

private:
  index_type m_first{};
  size_type m_size{};
};

auto size(const index_range& range) noexcept {
  return range.size();
}
bool empty(const index_range& range) noexcept {
  return size(range) == 0;
}
auto begin(const index_range& range) noexcept {
  return range.begin();
}
auto end(const index_range& range) noexcept {
  return begin(range) + size(range);
}
auto before_end(const index_range& range) noexcept {
  assert(size(range) >= 0);
  return end(range) - 1;
}

auto make_lhs_range(const node_proxy& node, const index_range& range) noexcept {
  assert(!empty(range));
  assert(begin(range) >= 0 && end(range) <= node.size());

  return index_range{exclusive_rank_0(node, begin(range)),
                     inclusive_rank_0(node, before_end(range))};
}

auto make_rhs_range(const node_proxy& node, const index_range& range) noexcept {
  assert(!empty(range));
  assert(begin(range) >= 0 && end(range) <= node.size());

  return index_range{exclusive_rank_1(node, begin(range)),
                     inclusive_rank_1(node, before_end(range))};
}

auto make_rhs_range_using_lhs(const index_range& range,
                              const index_range& lhs_range) {
  assert(begin(range) >= begin(lhs_range));
  assert(end(range) >= end(lhs_range));
  return index_range{begin(range) - begin(lhs_range),
                     end(range) - end(lhs_range)};
}

auto make_lhs_and_rhs_ranges(const node_proxy& node, const index_range& range) {
  assert(begin(range) >= 0 && end(range) <= node.size());

  const auto lhs = make_lhs_range(node, range);
  const auto rhs = make_rhs_range_using_lhs(range, lhs);
  return std::make_pair(lhs, rhs);
}

} // end anonymous namespace

// ==========================================
// wavelet_tree extensions
// ==========================================

static size_type range_rank(const wavelet_tree& wt, const symbol_id symbol,
                            index_type pos) noexcept {
  size_type count = 0;
  auto node = wt.make_root();
  while (!node.is_leaf()) {
    if (node.is_lhs_symbol(symbol)) {
      pos = node.rank_0(pos) - 1;
      if (pos == -1) {
        return count;
      }
      node = node.make_lhs();
    } else {
      count += node.rank_0(pos);
      pos = node.rank_1(pos) - 1;
      if (pos == -1) {
        return count;
      }
      node = node.make_rhs();
    }
  }
  if (node.is_lhs_symbol(symbol)) {
    count += node.rank_0(pos);
  } else {
    count += (pos + 1);
  }
  return count;
}

// Returns the element that would occur in the nth position in the given range
// if it was sorted.
//
static std::pair<symbol_id, index_type>
nth_element(const wavelet_tree& wt, index_range range, size_type nth) noexcept {

  assert(nth > 0 && nth <= size(range));
  const auto root_begin = begin(range);

  symbol_id symbol{};
  auto node = wt.make_root();
  while (!node.is_leaf()) {
    const auto lhs_range = make_lhs_range(node, range);
    const auto rhs_range = make_rhs_range_using_lhs(range, lhs_range);

    if (nth <= size(lhs_range)) {
      range = lhs_range;
      node = node.make_lhs();
      // nth -= 0
      // symbol |= 0;
    } else {
      range = rhs_range;
      node = node.make_rhs();
      nth -= size(lhs_range);
      symbol |= 1;
    }
    symbol <<= 1;
  }
  {
    const auto lhs_range = make_lhs_range(node, range);
    if (nth > size(lhs_range)) {
      nth -= size(lhs_range);
      symbol |= 1;
    }
  }
  assert(nth > 0);

  const auto abs_nth = nth + exclusive_rank(wt, symbol, root_begin);
  return std::make_pair(symbol, wt.select(symbol, abs_nth));
}

namespace count_symbols_detail {

// TODO(diego): Optimization. Some overloads of count_symbols generate children
// nodes even when its respective ranges are empty.

namespace {
struct greater_equal {
  symbol_id symbol;
};

struct less_equal {
  symbol_id symbol;
};
} // namespace

static size_type count_symbols(node_proxy node, const index_range range) {
  assert(begin(range) >= 0 && end(range) <= node.size());

  if (empty(range)) {
    return 0;
  }

  const auto children_ranges = make_lhs_and_rhs_ranges(node, range);
  const auto& lhs_range = get_left(children_ranges);
  const auto& rhs_range = get_right(children_ranges);

  if (!node.is_leaf()) {
    if (empty(lhs_range)) {
      return count_symbols(node.make_rhs(), rhs_range);
    }
    if (empty(rhs_range)) {
      return count_symbols(node.make_lhs(), lhs_range);
    }
    const auto children = node.make_lhs_and_rhs();
    return count_symbols(get_left(children), lhs_range) +
           count_symbols(get_right(children), rhs_range);
  }

  return (empty(lhs_range) ? 0 : 1) + (empty(rhs_range) ? 0 : 1);
}

static size_type count_symbols(node_proxy node, index_range range,
                               const greater_equal cond) noexcept {
  assert(begin(range) >= 0 && end(range) <= node.size());

  if (empty(range)) {
    return 0;
  }
  const auto min_symbol = cond.symbol;
  size_type count = 0;
  while (!node.is_leaf()) {
    const auto lhs_range = make_lhs_range(node, range);
    const auto rhs_range = make_rhs_range_using_lhs(range, lhs_range);

    if (node.is_lhs_symbol(min_symbol)) {
      const auto children = node.make_lhs_and_rhs(); // [lhs, rhs]
      count += count_symbols(get_right(children), rhs_range);
      node = get_left(children);
      range = lhs_range;
    } else {
      node = node.make_rhs();
      range = rhs_range;
    }

    if (empty(range)) {
      return count;
    }
  }
  assert(!empty(range));

  // At this point, the symbol of the rhs child is always >= min_symbol
  const auto rhs_range = make_rhs_range(node, range);
  if (!empty(rhs_range)) {
    count += 1;
  }
  if (node.is_lhs_symbol(min_symbol) && size(rhs_range) < size(range)) {
    count += 1; // the lhs range contains a valid symbol and is not empty.
  }
  return count;
}

static size_type count_symbols(node_proxy node, index_range range,
                               const less_equal cond) noexcept {
  assert(begin(range) >= 0 && end(range) <= node.size());

  if (empty(range)) {
    return 0;
  }
  const auto max_symbol = cond.symbol;
  size_type count = 0;
  while (!node.is_leaf()) {
    const auto lhs_range = make_lhs_range(node, range);
    const auto rhs_range = make_rhs_range_using_lhs(range, lhs_range);

    if (node.is_rhs_symbol(max_symbol)) {
      const auto children = node.make_lhs_and_rhs(); // [lhs, rhs]
      count += count_symbols(get_left(children), lhs_range);
      node = get_right(children);
      range = rhs_range;
    } else {
      node = node.make_lhs();
      range = lhs_range;
    }

    if (empty(range)) {
      return count;
    }
  }
  assert(!empty(range));

  // At this point, the symbol of the lhs child is always <= max_symbol
  const auto lhs_range = make_lhs_range(node, range);
  if (!empty(lhs_range)) {
    count += 1;
  }
  if (node.is_rhs_symbol(max_symbol) && size(lhs_range) < size(range)) {
    count += 1; // the rhs range contains a valid symbol and is not empty.
  }
  return count;
}

static size_type count_symbols(node_proxy node, index_range range,
                               const symbol_id min_symbol,
                               const symbol_id max_symbol) noexcept {

  assert(begin(range) >= 0 && end(range) <= node.size());
  assert(min_symbol <= max_symbol);

  if (empty(range)) {
    return 0;
  }

  while (!node.is_leaf()) {
    if (node.is_lhs_symbol(max_symbol)) {
      range = make_lhs_range(node, range);
      node = node.make_lhs();
    } else if (node.is_rhs_symbol(min_symbol)) {
      range = make_rhs_range(node, range);
      node = node.make_rhs();
    } else {
      assert(node.is_lhs_symbol(min_symbol) && node.is_rhs_symbol(max_symbol));

      const auto lhs_range = make_lhs_range(node, range);
      const auto rhs_range = make_rhs_range_using_lhs(range, lhs_range);
      const auto children = node.make_lhs_and_rhs();

      return count_symbols(get_left(children), lhs_range,
                           greater_equal{min_symbol}) +
             count_symbols(get_right(children), rhs_range,
                           less_equal{max_symbol});
    }

    if (empty(range)) {
      return 0;
    }
  }
  assert(!empty(range));
  assert(node.is_leaf());

  if (node.is_lhs_symbol(max_symbol)) {
    return empty(make_lhs_range(node, range)) ? 0 : 1;
  }
  if (node.is_rhs_symbol(min_symbol)) {
    return empty(make_rhs_range(node, range)) ? 0 : 1;
  }
  assert(node.is_lhs_symbol(min_symbol) && node.is_rhs_symbol(max_symbol));

  const auto lhs_range = make_lhs_range(node, range);
  size_type result = 0;
  if (!empty(lhs_range)) {
    result += 1; // lhs range is not empty
  }
  if (size(lhs_range) < size(range)) {
    result += 1; // rhs range is not empty
  }
  return result;
}

} // namespace count_symbols_detail

static size_type count_distinct_symbols(const wavelet_tree& wt,
                                        const index_range& range,
                                        const symbol_id min_symbol,
                                        const symbol_id max_symbol) {
  assert(begin(range) >= 0 && end(range) <= wt.size());
  namespace detail = count_symbols_detail;
  return detail::count_symbols(wt.make_root(), range, min_symbol, max_symbol);
}

// ==========================================
// binary_relation implementation
// ==========================================

static constexpr object_id prev(const object_id x) noexcept {
  assert(to_underlying_type(x) != 0);
  return static_cast<object_id>(to_underlying_type(x) - 1);
}

static constexpr label_id prev(const label_id alpha) noexcept {
  assert(to_underlying_type(alpha) != 0);
  return static_cast<label_id>(to_underlying_type(alpha));
}

namespace pairs_constructor_detail {
using std::vector;

static auto count_objects_frequency(const vector<pair_type>& pairs,
                                    const object_id max_object) {
  // TODO(Diego): Consider to replace the histogram with a compressed vector.
  const auto num_objects = static_cast<size_t>(max_object) + 1;
  std::vector<size_type> frequency(num_objects);
  for_each(pairs, [&](const pair_type& p) { ++frequency[p.object]; });
  return frequency;
}

static bitmap make_bitmap(const vector<size_type>& objects_frequency,
                          const size_type num_pairs) {
  auto bit_seq = [&] {
    const auto num_objects = static_cast<size_type>(objects_frequency.size());
    return bit_vector(/*count=*/num_pairs + num_objects);
  }();

  size_type acc_count = 0;
  for_each(objects_frequency, [&](const size_type count) {
    acc_count += count;
    bit_seq.set(acc_count, true);
    acc_count += 1;
  });
  return bitmap(std::move(bit_seq));
}

template <typename ForwardRange, typename T>
static void inplace_exclusive_scan(ForwardRange& range, const T init) {
  exclusive_scan(std::begin(range), std::end(range), std::begin(range), init);
}

static wavelet_tree make_wavelet_tree(const vector<pair_type>& pairs,
                                      vector<size_type> objects_frequency,
                                      const label_id max_label) {
  int_vector seq(/*count=*/static_cast<size_type>(pairs.size()),
                 /*bpe=*/used_bits(types::word_type{max_label}));

  // The exclusive_scan and the for_each are an inline counting sort.
  inplace_exclusive_scan(objects_frequency, size_type{0});
  for_each(pairs, [&](const pair_type& p) {
    const auto next_pos = objects_frequency[p.object]++;
    seq[next_pos] = p.label;
  });

  assert(objects_frequency.back() == seq.size());

  return wavelet_tree(std::move(seq));
}
} // end namespace pairs_constructor_detail

binary_relation::binary_relation(const std::vector<pair_type>& pairs) {
  object_id max_object{};
  label_id max_label{};
  for_each(pairs, [&](const pair_type& pair) {
    max_object = std::max(max_object, pair.object);
    max_label = std::max(max_label, pair.label);
  });

  namespace detail = pairs_constructor_detail;
  const auto num_pairs = static_cast<size_type>(pairs.size());
  const auto objects_frequency =
      detail::count_objects_frequency(pairs, max_object);

  m_bitmap = detail::make_bitmap(objects_frequency, num_pairs);
  m_wtree =
      detail::make_wavelet_tree(pairs, std::move(objects_frequency), max_label);
}

auto binary_relation::rank(object_id max_object, label_id max_label) const
    noexcept -> size_type {
  const auto end_pos = map(max_object);
  return range_rank(m_wtree, max_label, end_pos);
}

auto binary_relation::rank(object_id min_object, object_id max_object,
                           label_id max_label) const noexcept -> size_type {
  if (min_object == 0) {
    return rank(max_object, max_label);
  }
  return rank(max_object, max_label) - rank(prev(min_object), max_label);
}

auto binary_relation::select_label_major(const label_id alpha,
                                         const object_id x, const object_id y,
                                         size_type nth) const noexcept
    -> pair_type {
  assert(x <= y);

  nth += (alpha > 0 ? rank(x, y, prev(alpha)) : 0);

  const index_type first = (x > 0 ? map(prev(x)) + 1 : 0);
  const index_type last = map(y) + 1; // Elem after last pair with object = y

  symbol_id symbol{};
  index_type pos{};
  std::tie(symbol, pos) = nth_element(m_wtree, index_range{first, last}, nth);
  return {unmap(pos), label_id(symbol)};
}

auto binary_relation::object_select(label_id fixed_label,
                                    object_id object_start, size_type nth) const
    noexcept -> object_id {
  const auto objects_before = [&] {
    const auto wt_pos = map(object_start);
    return exclusive_rank(m_wtree, fixed_label, wt_pos);
  }();
  const auto wt_pos = m_wtree.select(fixed_label, nth + objects_before);
  return unmap(wt_pos);
}

auto binary_relation::count_distinct_labels(const label_id alpha,
                                            const label_id beta,
                                            const object_id x,
                                            const object_id y) const noexcept
    -> size_type {
  const auto range = index_range{map(x), map(y) + 1};
  return count_distinct_symbols(m_wtree, range, alpha, beta);
}

auto binary_relation::map(const object_id x) const noexcept -> index_type {
  const auto pos_of_one = m_bitmap.select_1(static_cast<size_type>(x) + 1);
  // Each object has at least an associated pair.
  assert(!m_bitmap.access(pos_of_one - 1));

  const auto wt_pos = m_bitmap.rank_0(pos_of_one) - 1;

  assert(wt_pos >= 0 && wt_pos < size());
  return wt_pos;
}

auto binary_relation::unmap(const index_type pos) const noexcept -> object_id {
  const auto pos_of_one = m_bitmap.select_0(pos + 1) + 1;
  assert(m_bitmap.access(pos_of_one)); // bit after last zero should be one.

  return static_cast<object_id>(m_bitmap.rank_1(pos_of_one) - 1);
}

} // end namespace brwt
