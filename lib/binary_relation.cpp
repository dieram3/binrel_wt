#include <brwt/binary_relation.h>

#include <cassert> // assert
#include <tuple>   // tie
#include <utility> // pair, make_pair

using brwt::types::size_type;
using brwt::types::index_type;

using brwt::binary_relation;

using brwt::wavelet_tree;
using node_proxy = wavelet_tree::node_proxy;
using symbol_id = wavelet_tree::symbol_id;

// ==========================================
// Helper classes and functions
// ==========================================

template <typename T>
static const T& get_left(const std::pair<T, T>& p) {
  return p.first;
}

template <typename T>
static const T& get_right(const std::pair<T, T>& p) {
  return p.second;
}

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
                                const index_type pos) {
  assert(pos >= 0 && pos <= wt.size());
  if (pos == 0) {
    return 0;
  }
  return wt.rank(symbol, pos);
}

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

[[deprecated]] static size_type rank_0(const node_proxy& node,
                                       const index_range range) {
  return inclusive_rank_0(node, before_end(range)) -
         exclusive_rank_0(node, begin(range));
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

namespace brwt {
namespace count_symbols_detail {
namespace {

// TODO(diego): Optimization. Some overloads of count_symbols generate children
// nodes even when its respective ranges are empty.

struct greater_equal {
  symbol_id symbol;
};

struct less_equal {
  symbol_id symbol;
};

size_type count_symbols(node_proxy node, const index_range range) {
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

size_type count_symbols(node_proxy node, index_range range,
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

size_type count_symbols(node_proxy node, index_range range,
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

size_type count_symbols(node_proxy node, index_range range,
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

} // end anonymous namespace
} // end namespace count_symbols_detail
} // end namespace brwt

static size_type count_distinct_symbols(const node_proxy& node,
                                        const index_range& range,
                                        const symbol_id min_symbol,
                                        const symbol_id max_symbol) {
  namespace detail = brwt::count_symbols_detail;
  return detail::count_symbols(node, range, min_symbol, max_symbol);
}

// ==========================================
// binary_relation implementation
// ==========================================

auto binary_relation::rank(label_type label_max, object_type obj_max) const
    noexcept -> size_type {
  const auto end_pos = map(obj_max);
  return range_rank(m_wtree, label_max, end_pos);
}

auto binary_relation::rank(label_type label_max, object_type obj_min,
                           object_type obj_max) const noexcept -> size_type {
  if (obj_min == 0) {
    return rank(label_max, obj_max);
  }
  return rank(label_max, obj_max) - rank(label_max, obj_min - 1);
}

auto binary_relation::select_label_major(const label_type alpha,
                                         const object_type x,
                                         const object_type y,
                                         size_type nth) const noexcept
    -> pair_type {
  assert(x <= y);

  nth += (alpha > 0 ? rank(alpha - 1, x, y) : 0);

  const index_type first = (x > 0 ? map(x - 1) + 1 : 0);
  const index_type last = map(y) + 1; // Elem after last pair with object = y

  label_type label{};
  index_type pos{};
  std::tie(label, pos) = nth_element(m_wtree, index_range{first, last}, nth);
  return {label, unmap(pos)};
}

auto binary_relation::object_select(label_type fixed_label, object_type obj_min,
                                    size_type nth) const noexcept
    -> object_type {
  const auto p = map(obj_min);
  const auto objects_before = p > 0 ? m_wtree.rank(fixed_label, p - 1) : 0;
  return unmap(m_wtree.select(fixed_label, nth + objects_before));
}

// auto binary_relation::count_distinct_labels(const label_type alpha,
//                                             const label_type beta,
//                                             const object_type x,
//                                             const object_type y) const
//                                             noexcept
//     -> size_type {
//   return count_distinct_symbols(m_wtree.make_root(), alpha, beta, map(x),
//                                 map(y));
// }

auto binary_relation::map(const object_type x) const noexcept -> index_type {
  const auto zero_pos = m_bitmap.select_0(x + 1);
  // Each object has at least an associated pair.
  assert(m_bitmap.access(zero_pos - 1));

  const auto wt_pos = m_bitmap.rank_1(zero_pos) - 1;

  assert(wt_pos >= 0 && wt_pos < size());
  return wt_pos;
}

auto binary_relation::unmap(const index_type pos) const noexcept
    -> object_type {
  const auto zero_pos = m_bitmap.select_1(pos + 1) + 1;
  assert(!m_bitmap.access(zero_pos)); // bit after last one should be zero.

  return m_bitmap.rank_0(zero_pos) - 1;
}
