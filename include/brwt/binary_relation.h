#ifndef BRWT_BINARY_RELATION_H
#define BRWT_BINARY_RELATION_H

#include <brwt/bitmap.h>       // bitmap
#include <brwt/common_types.h> // size_type, word_type, optional
#include <brwt/wavelet_tree.h> // wavelet_tree
#include <vector>              // vector

namespace brwt {

// helper types

struct object_major_order_t {};
struct label_major_order_t {};

constexpr object_major_order_t obj_major{};
constexpr label_major_order_t lab_major{};

// class definition

class binary_relation {
public:
  // member types
  using size_type = types::size_type;
  enum object_id : types::size_type {};
  enum label_id : types::size_type {};

  struct pair_type {
    // TODO(Diego): Consider renaming pair_type.
    object_id object;
    label_id label;
  };

  /// \brief Creates an empty binary relation.
  ///
  binary_relation() = default;

  /// \brief Constructs a binary relation with the given sequence of pairs.
  ///
  /// \post The number of objects will be equal to the max object plus 1.
  /// \post The number of labels will be equal to the max label plus 1.
  ///
  explicit binary_relation(const std::vector<pair_type>& pairs);

  /// \name Relation view
  /// @{

  /// \brief Counts the number of pairs in the specified range.
  ///
  size_type rank(object_id max_object, label_id max_label) const noexcept;

  /// \brief Counts the number of pairs in the specified range.
  ///
  size_type rank(object_id min_object, object_id max_object,
                 label_id max_label) const noexcept;

  /// \brief Counts the number of pairs in the specified range.
  ///
  size_type rank(object_id max_object, label_id min_label,
                 label_id max_label) const noexcept;

  /// \brief Finds the nth pair in the range
  /// 'access(alpha, max_label, x, y)' when it is ordered by object value, then
  /// by label value (object major order).
  ///
  /// \remark In the literature this operation is known as \e rel_sel_obj_maj
  ///
  optional<pair_type> nth_element(object_id x, label_id alpha, label_id beta,
                                  size_type nth,
                                  object_major_order_t order) const noexcept;

  /// \brief Finds the nth pair in the range
  /// 'access(alpha, max_label, x, y)' when it is ordered by label value, then
  /// by object value (label major order).
  ///
  /// \remark In the literature this operation is known as \e rel_sel_lab_maj
  ///
  optional<pair_type> nth_element(object_id x, object_id y, label_id alpha,
                                  size_type nth,
                                  label_major_order_t order) const noexcept;

  /// \brief Finds the first relation 'r' greater than or equal to 'rel_start'
  /// in the range of all relations with label in [alpha, beta] such that the
  /// range is ordered in object major order.
  ///
  /// \remark In the literature this operation is known as \e rel_min_obj_maj
  ///
  pair_type lower_bound_object_major(label_id alpha, label_id beta,
                                     pair_type pair_start) const noexcept;
  /// @}

  /// \name Object view
  /// @{

  /// \brief Returns the number of objects less than or equal to \p x that are
  /// associated with the given label.
  ///
  /// \remark This operation is also known as \c obj_rnk1.
  ///
  size_type obj_rank(object_id x, label_id fixed_label) const noexcept;

  /// \brief Returns the number of objects less than \p x that  are associated
  /// with the given label.
  ///
  size_type obj_exclusive_rank(object_id x, label_id fixed_label) const
      noexcept;

  /// \brief Returns the number of objects less than or equal to \p x that are
  /// associated with a label in the given range.
  ///
  /// \remark This operation is also known as <tt>obj_rnk</tt>.
  ///
  size_type obj_rank(object_id x, label_id min_label, label_id max_label) const
      noexcept;

  /// \brief Returns the number of objects less than \p x that are associated
  /// with a label in the given range.
  ///
  size_type obj_exclusive_rank(object_id x, label_id min_label,
                               label_id max_label) const noexcept;

  /// \brief Returns the \e nth smallest object associated with the given label,
  /// not less than the \e start object.
  ///
  /// Technically, returns the \e nth smallest object in the range
  /// \c obj_access(fixed_label, fixed_label, object_start, n).
  ///
  /// \pre <tt>nth > 0</tt>
  ///
  /// \returns The \e nth object if it exists, otherwise returns \c nullopt.
  ///
  /// \remark This operation is also known as \c obj_sel1.
  ///
  optional<object_id> obj_select(object_id object_start, label_id fixed_label,
                                 size_type nth) const noexcept;
  /// @}

  /// \name Label view
  /// @{

  /// \brief Count the number of labels in the range lab_acc(alpha, beta, x, y).
  ///
  /// \remark This operation is also known as \c lab_num.
  ///
  size_type count_distinct_labels(object_id x, object_id y, label_id alpha,
                                  label_id beta) const noexcept;
  /// @}

  /// \name Miscellaneous
  /// @{

  /// \brief Returns the number of pairs in the binary relation.
  ///
  size_type size() const noexcept;

  /// \brief Returns the size of the object alphabet.
  ///
  [[deprecated]] size_type num_objects() const noexcept;

  /// \brief Returns the size of the object alphabet.
  ///
  size_type object_alphabet_size() const noexcept;

  /// \brief Returns the size of the label alphabet.
  ///
  size_type label_alphabet_size() const noexcept;

  /// @}

private:
  index_type map(object_id x) const noexcept;
  object_id unmap(index_type wt_pos) const noexcept;

  index_type lower_bound(object_id x) const noexcept;
  index_type upper_bound(object_id x) const noexcept;
  auto make_mapped_range(object_id x, object_id y) const noexcept;
  object_id get_associated_object(index_type wt_pos) const noexcept;

  // member data
  wavelet_tree m_wtree;
  bitmap m_bitmap;
};

// ==========================================
// Inline definitions
// ==========================================

inline auto binary_relation::size() const noexcept -> size_type {
  return m_wtree.size();
}

inline auto binary_relation::num_objects() const noexcept -> size_type {
  return object_alphabet_size();
}

inline auto binary_relation::object_alphabet_size() const noexcept
    -> size_type {
  return m_bitmap.length() - size();
}

inline auto binary_relation::label_alphabet_size() const noexcept -> size_type {
  return m_wtree.max_symbol_id() + 1;
}

// ==========================================
// Non-member helpers
// ==========================================

/// \brief Compares \p lhs and \p rhs for equality.
///
/// \returns \c true if the respective parts of \c lhs and \c rhs are equal, \c
/// false otherwise.
///
/// \relates binary_relation::pair_type
///
constexpr bool operator==(const binary_relation::pair_type& lhs,
                          const binary_relation::pair_type& rhs) noexcept {
  return lhs.object == rhs.object && lhs.label == rhs.label;
}

/// \brief Compares \p lhs and \p rhs for inequality.
///
/// \returns <tt>!(lhs == rhs)</tt>
///
/// \relates binary_relation::pair_type
///
constexpr bool operator!=(const binary_relation::pair_type& lhs,
                          const binary_relation::pair_type& rhs) noexcept {
  return !(lhs == rhs);
}

} // end namespace brwt

#endif // BRWT_BINARY_RELATION_H
