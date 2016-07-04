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

  // This temporal histogram uses roughly the same space than pointers would.
  const auto sigma = static_cast<std::size_t>(alphabet_size);
  std::vector<size_type> next_pos(2 * sigma);

  for (size_type i = 0; i < sequence.length(); ++i) {
    const auto symbol = sequence[i];
    ++next_pos[sigma + symbol];
  }
  {
    const auto first = next_pos.begin() + alphabet_size;
    exclusive_scan(first, next_pos.end(), first, size_type{0});
  }
  for (auto j = sigma - 1; j > 0; --j) {
    const auto lhs = 2 * j; // rhs would be (2 * j + 1)
    next_pos[j] = next_pos[lhs];
  }

  // Finally we can fill the table.
  bit_vector bit_seq(sequence.get_bpe() * seq_len);
  auto push_symbol = [&](const symbol_type symbol) {
    symbol_type j = 1;
    symbol_type base_symbol = 0;
    auto num_symbols = sigma;
    size_type level_pos = 0;

    while (num_symbols > 1) {
      const auto lhs_symbols = num_symbols / 2;
      const bool is_lhs = (symbol < base_symbol + lhs_symbols);

      bit_seq.set(level_pos + (next_pos[j]++), !is_lhs);

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
    assert(j == sigma + symbol);
    assert(base_symbol == symbol);
    assert(num_symbols == 1);
    assert(level_pos == bit_seq.length());
  };
  for (size_type i = 0; i < sequence.length(); ++i) {
    push_symbol(sequence[i]);
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

auto wavelet_tree::access(index_type pos) const noexcept -> symbol_type {
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

auto wavelet_tree::make_root() const noexcept -> node_desc {
  return node_desc{/*range_begin=*/0,
                   /*range_size=*/seq_len,
                   /*base_symbol=*/0,
                   /*num_symbols=*/alphabet_size,
                   /*ones_before=*/0};
}

auto wavelet_tree::make_lhs(const node_desc& node) const noexcept -> node_desc {
  const auto first = node.range_begin + seq_len;
  return node_desc{/*range_begin=*/first,
                   /*range_size=*/count_zeros(node),
                   /*base_symbol=*/node.base_symbol,
                   /*num_symbols=*/node.num_symbols / 2,
                   /*ones_before=*/table.rank_1(first - 1)};
}

auto wavelet_tree::make_rhs(const node_desc& node) const noexcept -> node_desc {
  const auto num_zeros = count_zeros(node);
  const auto mid_ns = node.num_symbols / 2;
  const auto first = node.range_begin + seq_len + num_zeros;
  return node_desc{
      /*range_begin=*/first,
      /*range_size=*/(node.range_size - num_zeros),
      /*base_symbol=*/(node.base_symbol + static_cast<symbol_type>(mid_ns)),
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
