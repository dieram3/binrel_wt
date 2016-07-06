#ifndef BRWT_WAVELET_TREE_H
#define BRWT_WAVELET_TREE_H

#include <brwt/bitmap.h>     // bitmap
#include <brwt/int_vector.h> // int_vector
#include <cstddef>           // ptrdiff_t

namespace brwt {

class wavelet_tree {
public:
  using symbol_id = bit_vector::block_type;
  using size_type = std::ptrdiff_t;
  using index_type = std::ptrdiff_t;

  class node_desc {
  public:
    explicit node_desc(const wavelet_tree& wt) noexcept;

    // internal bitmap access
    bool access(size_type pos) const noexcept;
    size_type rank_0(index_type pos) const noexcept;
    size_type rank_1(index_type pos) const noexcept;
    index_type select_0(size_type nth) const noexcept;
    index_type select_1(size_type nth) const noexcept;
    size_type size() const noexcept;

    // Level information
    bool is_leaf() const noexcept;
    bool is_lhs_symbol(symbol_id symbol) const noexcept;

    // Navigation
    node_desc make_lhs() const noexcept;
    node_desc make_rhs() const noexcept;

  private:
    // Memberwise constructor
    node_desc(const wavelet_tree& wt_, index_type begin_, size_type size_,
              size_type ones_before_, symbol_id level_mask_) noexcept;

    // absolute position information
    index_type begin() const noexcept;
    index_type end() const noexcept;
    size_type zeros_before() const noexcept;
    size_type ones_before() const noexcept;

    // Auxiliary methods
    size_type count_zeros() const noexcept;
    const bitmap& get_table() const noexcept;

  private:
    const wavelet_tree* wt_ptr;
    index_type range_begin;
    size_type range_size;
    size_type num_ones_before;
    symbol_id level_mask;
  };
  friend node_desc;

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
  symbol_id access(index_type pos) const noexcept;

  /// \brief Counts how many occurrences has a symbol up to the given position.
  ///
  /// \pre <tt>symbol < get_alphabet_size()</tt>
  /// \pre <tt>pos < length()</tt>
  ///
  size_type rank(symbol_id symbol, index_type pos) const noexcept;

  /// \brief Finds the position of the \e nth occurrence of the given symbol.
  ///
  /// \pre <tt>symbol < get_alphabet_size()</tt>
  /// \pre <tt>nth > 0</tt>
  ///
  /// \returns The position of the \e nth symbol if it exists. Otherwise returns
  /// <tt>-1</tt>.
  ///
  index_type select(symbol_id symbol, size_type nth) const noexcept;

  /// \brief Gets the size (or length) of the original sequence.
  ///
  size_type size() const noexcept;

  /// \brief Gets the number of bits per symbol used in this wavelet tree.
  ///
  int get_bits_per_symbol() const noexcept;

  /// \brief Returns the maximum symbol id representable for this wavelet tree.
  ///
  symbol_id max_symbol_id() const noexcept;

  /// \brief Creates the root node.
  ///
  /// The returned node allows navigate through the wavelet tree.
  ///
  node_desc make_root() const noexcept;

private:
  bitmap table{};      // Representation of the wavelet tree without pointers.
  size_type seq_len{}; // The length of the original sequence.
  size_type bits_per_symbol{}; // The number of bits per symbol used in *this.
};

// ==========================================
// Inline definitions
// ==========================================

inline auto wavelet_tree::size() const noexcept -> size_type {
  return seq_len;
}

inline auto wavelet_tree::make_root() const noexcept -> node_desc {
  return node_desc(*this);
}

} // end namespace brwt

#endif // BRWT_WAVELET_TREE_H
