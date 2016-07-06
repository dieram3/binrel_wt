#ifndef BRWT_WAVELET_TREE_H
#define BRWT_WAVELET_TREE_H

#include <brwt/bitmap.h>     // bitmap
#include <brwt/int_vector.h> // int_vector
#include <cstddef>           // ptrdiff_t

namespace brwt {

class wavelet_tree {
public:
  /// Unsigned type used to represent symbol codes.
  using symbol_id = bit_vector::block_type;

  /// Signed vocabulary type used to indicate quantities.
  using size_type = std::ptrdiff_t;

  /// Signed vocabulary type used to indicate positions.
  using index_type = size_type;

  class node_desc;

public:
  /// \brief Constructs an empty wavelet tree.
  ///
  wavelet_tree() = default;

  /// \brief Constructs a wavelet tree from the given sequence.
  ///
  /// \param sequence The input sequence.
  ///
  /// \post <tt>get_bits_per_symbol() == sequence.get_bpe()</tt>
  ///
  /// \par Complexity
  /// Given:
  /// \li <tt>bpe = sequence.get_bpe()</tt>
  /// \li <tt>n = sequence.length()</tt>
  /// \li <tt>sigma = 2<sup>bpe</sup></tt>
  ///
  /// The time and space complexity to build the wavelet tree using this
  /// constructor is <tt>O(bpe * n + sigma)</tt>.
  ///
  explicit wavelet_tree(const int_vector& sequence);

  /// \brief Retrieves the symbol at the given position.
  ///
  /// \pre <tt>pos < size()</tt>
  ///
  symbol_id access(index_type pos) const noexcept;

  /// \brief Counts how many occurrences has a symbol up to the given position.
  ///
  /// \pre <tt>symbol <= max_symbol_id()</tt>
  /// \pre <tt>pos >= 0 && pos < size()</tt>
  ///
  size_type rank(symbol_id symbol, index_type pos) const noexcept;

  /// \brief Finds the position of the \e nth occurrence of the given symbol.
  ///
  /// \pre <tt>symbol <= max_symbol_id()</tt>
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
  /// The returned node allows navigating through the wavelet tree.
  ///
  node_desc make_root() const noexcept;

private:
  /// Representation of the wavelet tree without pointers.
  bitmap table{};

  /// The length of the original sequence.
  size_type seq_len{};

  /// The number of bits per symbol used in <tt>*this</tt>.
  size_type bits_per_symbol{};
};

/// \brief Proxy class to access the nodes of a wavelet tree.
///
/// Note that the node descriptors allow exploring the wavelet tree regardless
/// of the internal structure.
///
class wavelet_tree::node_desc {
public:
  /// \brief Constructs the root node of the given wavelet tree.
  ///
  explicit node_desc(const wavelet_tree& wt) noexcept;

  // internal bitmap access

  /// \brief Retrieves the specified bit from this node bitmap.
  ///
  bool access(size_type pos) const noexcept;

  /// \brief Invokes \c rank_0 on this node bitmap.
  ///
  size_type rank_0(index_type pos) const noexcept;

  /// \brief Invokes \c rank_1 on this node bitmap.
  ///
  size_type rank_1(index_type pos) const noexcept;

  /// \brief Invokes \c select_0 on this node bitmap.
  ///
  index_type select_0(size_type nth) const noexcept;

  /// \brief Invokes \c select_1 on this node bitmap.
  ///
  index_type select_1(size_type nth) const noexcept;

  /// \brief Retrieves the size of this node bitmap.
  ///
  size_type size() const noexcept;

  // Level information

  /// \brief Checks whether the node has no materialized children.
  ///
  bool is_leaf() const noexcept;

  /// \brief Checks if the next bit of the symbol is handled by the left child.
  ///
  /// Note that this function can be called even on a leaf node because it does
  /// not need its children to be materialized.
  ///
  bool is_lhs_symbol(symbol_id symbol) const noexcept;

  // Navigation

  /// \brief Constructs a proxy to the left hand side child.
  ///
  /// \pre <tt>!is_leaf()</tt>
  ///
  node_desc make_lhs() const noexcept;

  /// \brief Constructs a proxy to the right hand side child.
  ///
  /// \pre <tt>!is_leaf()</tt>
  ///
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
