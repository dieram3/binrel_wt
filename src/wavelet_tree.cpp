#include <brwt/wavelet_tree.h>

#include <cassert>     // assert
#include <cstddef>     // size_t
#include <type_traits> // aligned_storage
#include <utility>     // move, forward, exchange
#include <vector>      // vector

using brwt::wavelet_tree;
using std::size_t;

// ==========================================
// Auxiliary algorithms
// ==========================================

template <typename InputIt, typename OutputIt, typename T>
static void exclusive_scan(InputIt first, const InputIt last, OutputIt d_first,
                           T init) {
  for (; first != last; ++first) {
    *d_first++ = std::exchange(init, init + (*first));
  }
}

// ==========================================
// static_vector (auxiliary class)
// ==========================================

namespace {
template <class T, size_t N>
class static_vector {
public:
  using const_pointer = const T*;
  using const_reference = const T&;

public:
  ~static_vector() {
    // If T is trivially destructible the compiler should optimize this away.
    while (!empty()) {
      pop_back();
    }
  }

  template <typename... Args>
  void emplace_back(Args&&... args) {
    assert(m_size < N);
    new (m_data + m_size) T(std::forward<Args>(args)...); // NOLINT
    ++m_size;
  }

  void pop_back() {
    assert(!empty());
    (data()[--m_size]).~T();
  }

  const_reference operator[](const size_t pos) const noexcept {
    assert(pos < m_size);
    return data()[pos];
  }

  const_reference back() const noexcept {
    assert(!empty());
    return data()[m_size - 1];
  }

  constexpr bool empty() const noexcept {
    return m_size == 0;
  }

  constexpr const_pointer data() const noexcept {
    return reinterpret_cast<const T*>(m_data); // NOLINT
  }

private:
  using aligned_storage_t =
      typename std::aligned_storage<sizeof(T), alignof(T)>::type;

  aligned_storage_t m_data[N];
  size_t m_size = 0;
};
} // end anonymous namespace

// ==========================================
// wavelet_vector implementation
// ==========================================

wavelet_tree::wavelet_tree(const int_vector& sequence)
    : table{},
      seq_len{sequence.length()},
      alphabet_size{size_type{1} << sequence.get_bpe()} {

  // This temporal histogram uses roughly the same space than pointers would.
  std::vector<size_type> next_pos(static_cast<size_t>(2 * alphabet_size));
  auto at = [](const size_type pos) { return static_cast<size_t>(pos); };

  for (size_type i = 0; i < sequence.length(); ++i) {
    const auto symbol = static_cast<symbol_id>(sequence[i]);
    ++next_pos[at(alphabet_size + symbol)];
  }
  {
    const auto first = next_pos.begin() + alphabet_size;
    exclusive_scan(first, next_pos.end(), first, size_type{0});
  }
  for (auto j = alphabet_size - 1; j > 0; --j) {
    const auto lhs = 2 * j; // rhs would be (2 * j + 1)
    next_pos[at(j)] = next_pos[at(lhs)];
  }

  // Finally we can fill the table.
  bit_vector bit_seq(sequence.get_bpe() * seq_len);
  auto push_symbol = [&](const symbol_id symbol) {
    symbol_id j = 1;
    symbol_id base_symbol = 0;
    auto num_symbols = alphabet_size;
    size_type level_pos = 0;

    while (num_symbols > 1) {
      const auto lhs_symbols = num_symbols / 2;
      const bool is_lhs = (symbol < base_symbol + lhs_symbols);

      bit_seq.set(level_pos + (next_pos[at(j)]++), !is_lhs);

      if (is_lhs) {
        j = 2 * j;
        // base_symbol = base_symbol; // does not change
        num_symbols = lhs_symbols;
      } else {
        j = 2 * j + 1;
        base_symbol = (base_symbol + lhs_symbols);
        num_symbols = (num_symbols - lhs_symbols);
      }
      level_pos += seq_len;
    }
    assert(j == alphabet_size + symbol);
    assert(base_symbol == symbol);
    assert(num_symbols == 1);
    assert(level_pos == bit_seq.length());
  };
  for (size_type i = 0; i < sequence.length(); ++i) {
    push_symbol(static_cast<symbol_id>(sequence[i]));
  }

  table = bitmap(std::move(bit_seq)); // The final magic.

  // [&] {
  //   std::clog << "DEBUG: Final table layout\n";
  //   for (size_type i = 0; i < sequence.length(); ++i) {
  //     std::clog << sequence[i];
  //   }
  //   std::clog << '\n';
  //   for (size_type i = 0; i < table.length(); ++i) {
  //     std::clog << table.access(i);
  //     if ((i + 1) % seq_len == 0) {
  //       std::clog << '\n';
  //     }
  //   }
  //   std::clog << '\n';
  // }();
}

auto wavelet_tree::access(index_type pos) const noexcept -> symbol_id {
  assert(pos >= 0 && pos < length());

  node_desc node = make_root();
  while (node.num_symbols > 2) {
    // each iteration invokes rank at most three times.
    if (!access(node, pos)) {
      pos = rank_0(node, pos) - 1; // 1 rank
      node = make_lhs(node);       // 1 rank
    } else {
      pos = rank_1(node, pos) - 1; // 1 rank
      node = make_rhs(node);       // 2 ranks
    }
  }
  assert(node.num_symbols == 2);
  return (!access(node, pos)) ? (node.base_symbol) : (node.base_symbol + 1);
}

auto wavelet_tree::rank(const symbol_id symbol, index_type pos) const noexcept
    -> size_type {
  assert(pos >= 0 && pos < length());
  assert(symbol >= 0 && symbol < alphabet_size);

  node_desc node = make_root();
  while (node.num_symbols > 2) {
    // each iteration invokes rank at most three times.
    if (is_lhs_symbol(node, symbol)) {
      pos = rank_0(node, pos) - 1; // 1 rank
      if (pos == -1) {
        return 0;
      }
      node = make_lhs(node); // 1 rank
    } else {
      pos = rank_1(node, pos) - 1; // 1 rank
      if (pos == -1) {
        return 0;
      }
      node = make_rhs(node); // 2 ranks
    }
  }
  assert(node.num_symbols == 2);

  return is_lhs_symbol(node, symbol) ? rank_0(node, pos) : rank_1(node, pos);
}

auto wavelet_tree::select(const symbol_id symbol, const size_type nth) const
    noexcept -> index_type {
  assert(nth > 0);
  // Time complexity: Exactly 2 bitmap ranks and 1 bitmap select for level.

  static_vector<node_desc, 64> stack;
  stack.emplace_back(make_root());
  while (true) {
    const auto& node = stack.back();
    if (size(node) < nth) {
      return -1;
    }
    if (node.num_symbols == 2) {
      break;
    }
    stack.emplace_back(is_lhs_symbol(node, symbol) ? make_lhs(node)
                                                   : make_rhs(node));
  }

  auto pos = [&] {
    const auto& node = stack.back();
    assert(size(node) >= nth);
    return is_lhs_symbol(node, symbol) ? select_0(node, nth)
                                       : select_1(node, nth);
  }();
  if (pos == -1) {
    return -1; // such element does not exists.
  }

  stack.pop_back();
  while (!stack.empty()) {
    const auto& node = stack.back();
    pos = is_lhs_symbol(node, symbol) ? select_0(node, pos + 1)
                                      : select_1(node, pos + 1);
    assert(pos >= 0 && pos < size(node)); // The element is supposed to exist.
    stack.pop_back();
  }
  return pos;
};

auto wavelet_tree::make_root() const noexcept -> node_desc {
  return node_desc{/*range_begin=*/0,
                   /*range_size=*/seq_len,
                   /*base_symbol=*/0,
                   /*num_symbols=*/alphabet_size,
                   /*ones_before=*/0};
}

// This function invokes rank_1 twice.
auto wavelet_tree::make_lhs(const node_desc& node) const noexcept -> node_desc {
  const auto first = node.range_begin + seq_len;
  return node_desc{/*range_begin=*/first,
                   /*range_size=*/count_zeros(node),
                   /*base_symbol=*/node.base_symbol,
                   /*num_symbols=*/node.num_symbols / 2,
                   /*ones_before=*/table.rank_1(first - 1)};
}

// This function invokes rank_1 twice.
auto wavelet_tree::make_rhs(const node_desc& node) const noexcept -> node_desc {
  const auto num_zeros = count_zeros(node);
  const auto mid_ns = node.num_symbols / 2;
  const auto first = node.range_begin + seq_len + num_zeros;
  return node_desc{/*range_begin=*/first,
                   /*range_size=*/(node.range_size - num_zeros),
                   /*base_symbol=*/(node.base_symbol + mid_ns),
                   /*num_symbols=*/(node.num_symbols - mid_ns),
                   /*ones_before=*/table.rank_1(first - 1)};
}

bool wavelet_tree::access(const node_desc& node, const index_type rel_pos) const
    noexcept {
  return table.access(node.range_begin + rel_pos);
}

// This function invokes rank_1 once.
auto wavelet_tree::rank_0(const node_desc& node, const index_type rel_pos) const
    noexcept -> size_type {
  assert(rel_pos >= 0 && rel_pos < size(node));
  const auto abs_pos = begin(node) + rel_pos;
  return table.rank_0(abs_pos) - zeros_before(node);
}

// This function invokes rank_1 once.
auto wavelet_tree::rank_1(const node_desc& node, const index_type rel_pos) const
    noexcept -> size_type {
  assert(rel_pos >= 0 && rel_pos < size(node));
  const auto abs_pos = begin(node) + rel_pos;
  return table.rank_1(abs_pos) - ones_before(node);
}

auto wavelet_tree::select_0(const node_desc& node, const size_type nth) const
    noexcept -> index_type {
  assert(nth > 0);
  const auto abs_pos = table.select_0(zeros_before(node) + nth);
  if (abs_pos == -1 || abs_pos >= end(node)) {
    return -1;
  }
  return abs_pos - begin(node);
}

auto wavelet_tree::select_1(const node_desc& node, const size_type nth) const
    noexcept -> index_type {
  assert(nth > 0);
  const auto abs_pos = table.select_1(ones_before(node) + nth);
  if (abs_pos == -1 || abs_pos >= end(node)) {
    return -1;
  }
  return abs_pos - begin(node);
}

auto wavelet_tree::count_zeros(const node_desc& node) const noexcept
    -> size_type {
  return rank_0(node, size(node) - 1);
}

constexpr auto wavelet_tree::begin(const node_desc& node) noexcept
    -> index_type {
  return node.range_begin;
}

constexpr auto wavelet_tree::end(const node_desc& node) noexcept -> index_type {
  return node.range_begin + node.range_size;
}

constexpr auto wavelet_tree::size(const node_desc& node) noexcept -> size_type {
  return node.range_size;
}

constexpr auto wavelet_tree::zeros_before(const node_desc& node) noexcept
    -> size_type {
  return node.range_begin - node.ones_before;
}

constexpr auto wavelet_tree::ones_before(const node_desc& node) noexcept
    -> size_type {
  return node.ones_before;
}

constexpr auto wavelet_tree::is_lhs_symbol(const node_desc& node,
                                           const symbol_id symbol) noexcept
    -> bool {
  assert(symbol >= node.base_symbol &&
         symbol < node.base_symbol + node.num_symbols);
  return symbol < (node.base_symbol + node.num_symbols / 2);
}
