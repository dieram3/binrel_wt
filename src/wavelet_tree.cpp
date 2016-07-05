#include <brwt/wavelet_tree.h>

#include <cassert> // assert
#include <utility> // move, exchange
#include <vector>  // vector

using brwt::wavelet_tree;

// ==========================================
// Auxiliary functions
// ==========================================

template <typename InputIt, typename OutputIt, typename T>
static void exclusive_scan(InputIt first, const InputIt last, OutputIt d_first,
                           T init) {
  for (; first != last; ++first) {
    *d_first++ = std::exchange(init, init + (*first));
  }
}

// ==========================================
// wavelet_vector implementation
// ==========================================

wavelet_tree::wavelet_tree(const int_vector& sequence)
    : table{},
      seq_len{sequence.length()},
      alphabet_size{size_type{1} << sequence.get_bpe()} {
  using std::size_t;

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
  if (alphabet_size == 1) {
    return 0;
  }
  node_desc node = make_root();
  while (node.num_symbols > 2) {
    // each iteration invokes rank at most three times.
    const auto absolute_pos = node.range_begin + pos;
    if (!table.access(absolute_pos)) {
      const auto zeros_before = (node.range_begin - node.ones_before);
      pos = (table.rank_0(absolute_pos) - zeros_before) - 1; // 1 rank
      node = make_lhs(node);                                 // 2 ranks
    } else {
      const auto ones_before = node.ones_before;
      pos = (table.rank_1(absolute_pos) - ones_before) - 1; // 1 rank
      node = make_rhs(node);                                // 2 ranks
    }
    assert(pos >= 0);
  }
  assert(node.num_symbols == 2);

  return table.access(node.range_begin + pos) ? (node.base_symbol + 1)
                                              : (node.base_symbol);
}

auto wavelet_tree::rank(const symbol_id symbol, index_type pos) const noexcept
    -> size_type {
  assert(pos >= 0 && pos < length());
  assert(symbol < static_cast<symbol_id>(alphabet_size));

  node_desc node = make_root();
  while (node.num_symbols > 2) {
    // each iteration invokes rank at most three times.
    const auto abs_pos = node.range_begin + pos;
    const auto mid_ns = static_cast<symbol_id>(node.num_symbols / 2);
    if (symbol < (node.base_symbol + mid_ns)) {
      // 'symbol' is a lhs symbol.
      const auto zeros_before = (node.range_begin - node.ones_before);
      const auto rank0 = table.rank_0(abs_pos) - zeros_before; // 1 rank
      if (rank0 == 0) {
        return 0;
      }
      pos = rank0 - 1;
      node = make_lhs(node); // 2 ranks
    } else {
      // 'symbol' is a rhs symbol.
      const auto ones_before = node.ones_before;
      const auto rank1 = table.rank_1(abs_pos) - ones_before; // 1 rank
      if (rank1 == 0) {
        return 0;
      }
      pos = rank1 - 1;       // 1 rank
      node = make_rhs(node); // 2 ranks
    }
    assert(pos >= 0);
  }
  assert(node.num_symbols == 2);

  const auto abs_pos = node.range_begin + pos;
  if (symbol != node.base_symbol) {
    // 'symbol' is rhs symbol.
    return table.rank_1(abs_pos) - node.ones_before;
  }
  const auto zeros_before = node.range_begin - node.ones_before;
  return table.rank_0(abs_pos) - zeros_before;
}

// auto wavelet_tree::select(const symbol_id symbol, const size_type nth) const
//     noexcept -> index_type {
//   return 0;
// };

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
  return node_desc{
      /*range_begin=*/first,
      /*range_size=*/(node.range_size - num_zeros),
      /*base_symbol=*/(node.base_symbol + static_cast<symbol_id>(mid_ns)),
      /*num_symbols=*/(node.num_symbols - mid_ns),
      /*ones_before=*/table.rank_1(first - 1)};
}

// This function invokes rank_1 once.
auto wavelet_tree::count_ones(const node_desc& node) const noexcept
    -> size_type {
  return table.rank_1(node.range_begin + node.range_size - 1) -
         node.ones_before;
}

// This function invokes rank_1 once.
auto wavelet_tree::count_zeros(const node_desc& node) const noexcept
    -> size_type {
  return node.range_size - count_ones(node);
}

// auto wavelet_tree::is_lhs_symbol(const node_desc& node,
//                                  const symbol_id symbol) const noexcept
//     -> bool {
//   assert(symbol >= node.base_symbol);
//   assert(symbol < node.base_symbol +
//   static_cast<symbol_id>(node.num_symbols));
//   const auto mid_ns = static_cast<symbol_id>(node.num_symbols / 2);
// }
