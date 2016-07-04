#ifndef BRWT_WAVELET_TREE_H
#define BRWT_WAVELET_TREE_H

#include <brwt/bitmap.h>
#include <brwt/int_vector.h>
#include <cstddef> // ptrdiff_t

namespace brwt {

class wavelet_tree {
public:
  using symbol_type = int_vector::value_type;
  using size_type = std::ptrdiff_t;
  using index_type = std::ptrdiff_t;

private:
  struct node_desc {
    index_type first;
    index_type last;
    symbol_type base_symbol;
    size_type num_symbols;
  };

public:
  /// \brief Constructs an empty wavelet tree.
  ///
  wavelet_tree() = default;

  /// \brief Constructs a wavelet tree from the given sequence.
  ///
  /// \param sequence The input sequence.
  ///
  /// \post <tt>get_alphabet_size() == pow(2, sequence.get_bpe())</tt>
  ///
  /// \remark The usage of memory of the wavelet tree will be roughly the same
  /// that the given sequence.
  ///
  explicit wavelet_tree(const int_vector& sequence);

  /// \brief Retrieves the symbol at the given position.
  ///
  /// \pre <tt>pos < length()</tt>
  ///
  symbol_type access(index_type pos) const noexcept;

  /// \brief Counts how many occurrences has a symbol up to the given position.
  ///
  /// \pre <tt>symbol < get_alphabet_size()</tt>
  /// \pre <tt>pos < length()</tt>
  ///
  size_type rank(symbol_type symbol, index_type pos) const noexcept;

  /// \brief Finds the position of the \e nth occurrence of the given symbol.
  ///
  /// \pre <tt>symbol < get_alphabet_size()</tt>
  /// \pre <tt>nth > 0</tt>
  ///
  /// \returns The position of the \e nth symbol if it exists. Otherwise returns
  /// <tt>-1</tt>.
  ///
  index_type select(symbol_type symbol, size_type nth) const noexcept;

  /// \brief Gets the length of the original sequence.
  ///
  size_type length() const noexcept;

  /// \brief Gets the size of the alpabhet tracked by this wavelet tree.
  ///
  size_type get_alphabet_size() const noexcept;

private:
  node_desc make_root() const noexcept;
  node_desc make_lhs(const node_desc& node) const noexcept;
  node_desc make_rhs(const node_desc& node) const noexcept;
  bool goes_to_lhs(const node_desc& node, symbol_type symbol) const noexcept;

private:
  bitmap table{};      // Representation of the wavelet tree without pointers.
  size_type seq_len{}; // The length of the original sequence.
  size_type alphabet_size{}; // a.k.a sigma
};

// ==========================================
// Inline definitions
// ==========================================

inline auto wavelet_tree::length() const noexcept -> size_type {
  return seq_len;
}

inline auto wavelet_tree::get_alphabet_size() const noexcept -> size_type {
  return alphabet_size;
}

} // end namespace brwt

#endif // BRWT_WAVELET_TREE_H
