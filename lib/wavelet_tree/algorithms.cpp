#include <brwt/wavelet_tree/algorithms.h>

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
                               const greater_equal cond) noexcept {
  assert(begin(range) >= 0 && end(range) <= node.size());

  if (empty(range)) {
    return 0;
  }
  const auto min_symbol = cond.symbol;
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
                               const less_equal cond) noexcept {
  assert(begin(range) >= 0 && end(range) <= node.size());

  if (empty(range)) {
    return 0;
  }
  const auto max_symbol = cond.symbol;
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
      symbol |= 1;
    }
    symbol <<= 1;
  }
  {
    const auto lhs_range = make_lhs_range(range, node);
    if (nth > size(lhs_range)) {
      nth -= size(lhs_range);
      symbol |= 1;
    }
  }
  assert(nth > 0);

  const auto abs_nth = nth + exclusive_rank(wt, symbol, root_begin);
  return std::make_pair(symbol, wt.select(symbol, abs_nth));
}

} // end namespace brwt
