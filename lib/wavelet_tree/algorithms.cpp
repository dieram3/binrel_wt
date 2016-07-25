#include <brwt/wavelet_tree/algorithms.h>

#include "../generic_algorithms.h"          // int_binary_search
#include "bitmask_support.h"                // symbol_id-stuff
#include <brwt/wavelet_tree/wavelet_tree.h> // wavelet_tree class definition.

namespace brwt {

using node_proxy = wavelet_tree::node_proxy;

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

// ==========================================
// index_range, node_proxy extensions
// ==========================================

static auto before_end(const index_range& range) noexcept {
  assert(size(range) > 0);
  return end(range) - 1;
}

static auto make_lhs_range(const index_range& range,
                           const node_proxy& node) noexcept {
  assert(!empty(range));
  assert(begin(range) >= 0 && end(range) <= node.size());

  return index_range{exclusive_rank_0(node, begin(range)),
                     inclusive_rank_0(node, before_end(range))};
}

static auto make_rhs_range(const index_range& range,
                           const node_proxy& node) noexcept {
  assert(!empty(range));
  assert(begin(range) >= 0 && end(range) <= node.size());

  return index_range{exclusive_rank_1(node, begin(range)),
                     inclusive_rank_1(node, before_end(range))};
}

static auto make_rhs_range_using_lhs(const index_range& range,
                                     const index_range& lhs_range) {
  assert(begin(range) >= begin(lhs_range));
  assert(end(range) >= end(lhs_range));
  return index_range{begin(range) - begin(lhs_range),
                     end(range) - end(lhs_range)};
}

static auto make_lhs_and_rhs_ranges(const index_range& range,
                                    const node_proxy& node) {
  assert(begin(range) >= 0 && end(range) <= node.size());

  const auto lhs = make_lhs_range(range, node);
  const auto rhs = make_rhs_range_using_lhs(range, lhs);
  return std::make_pair(lhs, rhs);
}

// ==========================================
// Wavelet Tree algorithms implementation
// ==========================================

size_type inclusive_rank(const wavelet_tree& wt, const symbol_id symbol,
                         const index_type pos) noexcept {
  return wt.rank(symbol, pos);
}

size_type exclusive_rank(const wavelet_tree& wt, const symbol_id symbol,
                         const index_type pos) noexcept {
  assert(pos >= 0 && pos <= wt.size());
  if (pos == 0) {
    return 0;
  }
  return inclusive_rank(wt, symbol, pos - 1);
}

size_type inclusive_rank(const wavelet_tree& wt,
                         const less_equal<symbol_id> cond,
                         index_type pos) noexcept {
  assert(pos >= 0 && pos < wt.size());

  const auto max_symbol = cond.max_value;
  size_type count = 0;
  auto node = wt.make_root();
  while (!node.is_leaf()) {
    if (node.is_lhs_symbol(max_symbol)) {
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
  if (node.is_lhs_symbol(max_symbol)) {
    count += node.rank_0(pos);
  } else {
    count += (pos + 1);
  }
  return count;
}

size_type exclusive_rank(const wavelet_tree& wt,
                         const less_equal<symbol_id> cond,
                         index_type pos) noexcept {
  assert(pos >= 0 && pos <= wt.size());
  if (pos == 0) {
    return 0;
  }
  return inclusive_rank(wt, cond, pos - 1);
}

size_type inclusive_rank(const wavelet_tree& wt, between<symbol_id> cond,
                         const index_type pos) noexcept {
  return exclusive_rank(wt, cond, pos + 1);
}

size_type exclusive_rank(const wavelet_tree& wt, between<symbol_id> cond,
                         index_type end_pos) noexcept {
  if (cond.min_value == 0) {
    return exclusive_rank(wt, less_equal<symbol_id>{cond.max_value}, end_pos);
  }
  return exclusive_rank(wt, less_equal<symbol_id>{cond.max_value}, end_pos) -
         exclusive_rank(wt, less_equal<symbol_id>{prev(cond.min_value)},
                        end_pos);
}

size_type rank(const wavelet_tree& wt, const index_range range,
               const between<symbol_id> cond) noexcept {
  // TODO(Diego): It can be implemented faster.
  return exclusive_rank(wt, cond, end(range)) -
         exclusive_rank(wt, cond, begin(range));
}

namespace count_symbols_detail {

// TODO(diego): Optimization. Some overloads of count_symbols generate children
// nodes even when its respective ranges are empty.

static size_type count_symbols(node_proxy node, const index_range range) {
  assert(begin(range) >= 0 && end(range) <= node.size());

  if (empty(range)) {
    return 0;
  }

  const auto children_ranges = make_lhs_and_rhs_ranges(range, node);
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
                               const greater_equal<symbol_id> cond) noexcept {
  assert(begin(range) >= 0 && end(range) <= node.size());

  if (empty(range)) {
    return 0;
  }
  const auto min_symbol = cond.min_value;
  size_type count = 0;
  while (!node.is_leaf()) {
    const auto lhs_range = make_lhs_range(range, node);
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
  const auto rhs_range = make_rhs_range(range, node);
  if (!empty(rhs_range)) {
    count += 1;
  }
  if (node.is_lhs_symbol(min_symbol) && size(rhs_range) < size(range)) {
    count += 1; // the lhs range contains a valid symbol and is not empty.
  }
  return count;
}

static size_type count_symbols(node_proxy node, index_range range,
                               const less_equal<symbol_id> cond) noexcept {
  assert(begin(range) >= 0 && end(range) <= node.size());

  if (empty(range)) {
    return 0;
  }
  const auto max_symbol = cond.max_value;
  size_type count = 0;
  while (!node.is_leaf()) {
    const auto lhs_range = make_lhs_range(range, node);
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
  const auto lhs_range = make_lhs_range(range, node);
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
      range = make_lhs_range(range, node);
      node = node.make_lhs();
    } else if (node.is_rhs_symbol(min_symbol)) {
      range = make_rhs_range(range, node);
      node = node.make_rhs();
    } else {
      assert(node.is_lhs_symbol(min_symbol) && node.is_rhs_symbol(max_symbol));

      const auto lhs_range = make_lhs_range(range, node);
      const auto rhs_range = make_rhs_range_using_lhs(range, lhs_range);
      const auto children = node.make_lhs_and_rhs();

      return count_symbols(get_left(children), lhs_range,
                           greater_equal<symbol_id>{min_symbol}) +
             count_symbols(get_right(children), rhs_range,
                           less_equal<symbol_id>{max_symbol});
    }

    if (empty(range)) {
      return 0;
    }
  }
  assert(!empty(range));
  assert(node.is_leaf());

  if (node.is_lhs_symbol(max_symbol)) {
    return empty(make_lhs_range(range, node)) ? 0 : 1;
  }
  if (node.is_rhs_symbol(min_symbol)) {
    return empty(make_rhs_range(range, node)) ? 0 : 1;
  }
  assert(node.is_lhs_symbol(min_symbol) && node.is_rhs_symbol(max_symbol));

  const auto lhs_range = make_lhs_range(range, node);
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

size_type count_distinct_symbols(const wavelet_tree& wt,
                                 const index_range range,
                                 const less_equal<symbol_id> cond) noexcept {
  assert(begin(range) >= 0 && end(range) <= wt.size());
  assert(cond.max_value >= 0 && cond.max_value <= wt.max_symbol_id());

  return count_symbols_detail::count_symbols(wt.make_root(), range, cond);
}

size_type count_distinct_symbols(const wavelet_tree& wt,
                                 const index_range range,
                                 const greater_equal<symbol_id> cond) noexcept {
  assert(begin(range) >= 0 && end(range) <= wt.size());
  assert(cond.min_value >= 0 && cond.min_value <= wt.max_symbol_id());

  return count_symbols_detail::count_symbols(wt.make_root(), range, cond);
}

size_type count_distinct_symbols(const wavelet_tree& wt,
                                 const index_range range,
                                 const between<symbol_id> cond) noexcept {
  assert(begin(range) >= 0 && end(range) <= wt.size());
  assert(cond.min_value >= 0 && cond.max_value <= wt.max_symbol_id());
  namespace detail = count_symbols_detail;
  return detail::count_symbols(wt.make_root(), range, cond.min_value,
                               cond.max_value);
}

std::pair<symbol_id, index_type>
nth_element(const wavelet_tree& wt, index_range range, size_type nth) noexcept {

  assert(nth > 0 && nth <= size(range));
  const auto root_begin = begin(range);

  symbol_id symbol{};
  auto node = wt.make_root();
  while (!node.is_leaf()) {
    const auto lhs_range = make_lhs_range(range, node);
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
      symbol |= 1u;
    }
    symbol <<= 1;
  }
  {
    const auto lhs_range = make_lhs_range(range, node);
    if (nth > size(lhs_range)) {
      nth -= size(lhs_range);
      symbol |= 1u;
    }
  }
  assert(nth > 0);

  const auto abs_nth = nth + exclusive_rank(wt, symbol, root_begin);
  return std::make_pair(symbol, wt.select(symbol, abs_nth));
}

index_type select(const wavelet_tree& wt, const between<symbol_id> cond,
                  const size_type nth) noexcept {
  // Find position such that:
  // inclusive_rank(wt, cond, pos) == nth;
  // exclusive_rank(wt, cond, pos) == nth - 1;
  auto pred = [&](const index_type pos) {
    return inclusive_rank(wt, cond, pos) < nth;
  };
  const auto pos = int_binary_search(index_type{0}, wt.size(), pred);
  if (pos == wt.size()) {
    return index_npos;
  }
  return pos;
}

namespace select_first_detail {

// ==========================
// index maps
// ==========================

static index_type make_lhs_pos(const node_proxy& node,
                               const index_type pos) noexcept {
  return exclusive_rank_0(node, pos);
}

static index_type make_rhs_pos(const node_proxy& node,
                               const index_type pos) noexcept {
  return exclusive_rank_1(node, pos);
}

static index_type map_rhs_pos(const node_proxy& node,
                              const index_type pos) noexcept {
  const auto nth = make_rhs_pos(node, pos) + 1;
  assert(nth > 0);
  return node.select_1(nth);
}

static index_type map_lhs_pos(const node_proxy& node,
                              const index_type pos) noexcept {
  const auto nth = make_lhs_pos(node, pos) + 1;
  assert(nth > 0);
  return node.select_0(nth);
}

// ==========================
// node_proxy extensions
// ==========================

static bool has_left_branch(const node_proxy& node) noexcept {
  return exclusive_rank_0(node, node.size()) > 0;
}

static bool has_right_branch(const node_proxy& node) noexcept {
  return exclusive_rank_1(node, node.size()) > 0;
}

// ==========================
// merge leaf divisions results
// ==========================

static index_type merge_leaf_div(const node_proxy& node, const index_type pos,
                                 const greater_equal<symbol_id> cond) noexcept {
  assert(node.is_leaf());

  const auto min_symbol = cond.min_value;
  if (node.is_lhs_symbol(min_symbol)) {
    return pos;
  } else if (has_right_branch(node)) {
    return map_rhs_pos(node, pos);
  }

  return index_npos;
}

static index_type merge_leaf_div(const node_proxy& node, const index_type pos,
                                 const less_equal<symbol_id> cond) noexcept {
  assert(node.is_leaf());

  const auto max_symbol = cond.max_value;
  if (node.is_rhs_symbol(max_symbol)) {
    return pos;
  } else if (has_left_branch(node)) {
    return map_lhs_pos(node, pos);
  }

  return index_npos;
}

// ==========================
// convenience functions
// ==========================
template <class SelectionFunction>
static index_type map_left_child(const node_proxy& node, const index_type pos,
                                 SelectionFunction&& fn) noexcept {
  const auto nth = fn(node.make_lhs(), make_lhs_pos(node, pos)) + 1;
  return nth == 0 ? index_npos : node.select_0(nth);
}

template <class SelectionFunction>
static index_type map_right_child(const node_proxy& node, index_type pos,
                                  SelectionFunction&& fn) noexcept {
  const auto nth = fn(node.make_rhs(), make_rhs_pos(node, pos)) + 1;
  return nth == 0 ? index_npos : node.select_1(nth);
}

// ==========================
// select first implementation
// ==========================

static index_type select_first(node_proxy node, index_type pos,
                               const greater_equal<symbol_id> cond) noexcept {
  assert(pos >= 0 && pos <= node.size());

  if (node.is_leaf()) {
    return merge_leaf_div(node, pos, cond);
  }

  auto select = [&](const node_proxy& n, index_type p) {
    return select_first(n, p, cond);
  };

  if (node.is_lhs_symbol(cond.min_value)) {
    if (has_left_branch(node)) {
      const auto mapped_rhs_pos = map_rhs_pos(node, pos);
      const auto mapped_lhs_pos = map_left_child(node, pos, select);

      if (mapped_rhs_pos != index_npos && mapped_lhs_pos != index_npos) {
        return std::min(mapped_lhs_pos, mapped_rhs_pos);
      }

      return mapped_rhs_pos == index_npos ? mapped_lhs_pos : mapped_rhs_pos;
    }

    return pos;
  }

  return map_right_child(node, pos, select);
}

static size_type select_first(node_proxy node, index_type pos,
                              const less_equal<symbol_id> cond) noexcept {
  assert(pos >= 0 && pos <= node.size());

  if (node.is_leaf()) {
    return merge_leaf_div(node, pos, cond);
  }

  auto select = [&](const node_proxy& n, index_type p) {
    return select_first(n, p, cond);
  };

  if (node.is_rhs_symbol(cond.max_value)) {
    if (has_right_branch(node)) {
      const auto mapped_lhs_pos = map_lhs_pos(node, pos);
      const auto mapped_rhs_pos = map_right_child(node, pos, select);

      if (mapped_lhs_pos != index_npos && mapped_rhs_pos != index_npos) {
        return std::min(mapped_lhs_pos, mapped_rhs_pos);
      }

      return mapped_rhs_pos == index_npos ? mapped_lhs_pos : mapped_rhs_pos;
    }

    return pos;
  }

  return map_left_child(node, pos, select);
}

static index_type select_first(node_proxy node, index_type pos,
                               const symbol_id min_symbol,
                               const symbol_id max_symbol) noexcept {
  assert(pos >= 0 && pos <= node.size());

  if (node.is_leaf()) {
    if (node.is_lhs_symbol(min_symbol) && node.is_rhs_symbol(max_symbol)) {
      return pos;
    }

    if (node.is_lhs_symbol(max_symbol)) {
      return map_lhs_pos(node, pos);
    }

    return map_rhs_pos(node, pos);
  }

  // check if interval [min_symbol, max_symbol] is fully cover
  // by either a left branch or a right branch.
  if (node.is_lhs_symbol(max_symbol)) {
    auto select = [&](const node_proxy& n, index_type p) {
      return select_first(n, p, min_symbol, max_symbol);
    };
    return map_left_child(node, pos, select);
  } else if (node.is_rhs_symbol(min_symbol)) {
    auto select = [&](const node_proxy& n, index_type p) {
      return select_first(n, p, min_symbol, max_symbol);
    };
    return map_right_child(node, pos, select);
  }

  // merge results from both branches
  auto select_ge = [&](const node_proxy& n, index_type p) {
    return select_first(n, p, greater_equal<symbol_id>{min_symbol});
  };
  auto select_le = [&](const node_proxy& n, index_type p) {
    return select_first(n, p, less_equal<symbol_id>{max_symbol});
  };

  const auto mapped_lhs_pos = map_left_child(node, pos, select_ge);
  const auto mapped_rhs_pos = map_right_child(node, pos, select_le);

  if (mapped_lhs_pos != index_npos && mapped_rhs_pos != index_npos) {
    assert(mapped_lhs_pos >= pos && mapped_rhs_pos >= pos);
    return std::min(mapped_lhs_pos, mapped_rhs_pos);
  }

  return mapped_lhs_pos == index_npos ? mapped_lhs_pos : mapped_rhs_pos;
}

} // end select_first_detail

index_type select_first(const wavelet_tree& wt, index_type start,
                        between<symbol_id> cond) noexcept {
  return select_first_detail::select_first(wt.make_root(), start,
                                           cond.min_value, cond.max_value);
}

} // end namespace brwt
