#ifndef BRWT_BINARY_RELATION_H
#define BRWT_BINARY_RELATION_H

#include <brwt/bitmap.h>
#include <brwt/common_types.h>
#include <brwt/int_vector.h>
#include <brwt/wavelet_tree.h>
#include <vector>

namespace brwt {

class binary_relation {
public:
  // types
  using size_type = types::size_type;
  enum object_id : types::word_type {};
  enum label_id : types::word_type {};

  struct pair_type {
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

  /// \brief Counts the number of pairs in the specified range.
  ///
  size_type rank(object_id max_object, label_id max_label) const noexcept;

  /// \brief Counts the number of pairs in the specified range.
  ///
  size_type rank(object_id min_object, object_id max_object,
                 label_id max_label) const noexcept;

  /// \brief Finds the nth pair in the range
  /// 'access(alpha, max_label, x, y)' when it is ordered by label value, then
  /// by object value (label major order).
  ///
  /// \remark In the literature this operation is known as \e rel_sel_lab_maj
  ///
  pair_type select_label_major(label_id alpha, object_id x, object_id y,
                               size_type nth) const noexcept;

  /// \brief TODO(diego)
  ///
  /// \remark In the literature this operation is known as \e rel_sel_obj_maj
  ///
  pair_type select_object_major(label_id alpha, label_id beta, object_id x,
                                size_type nth) const noexcept;

  /// \brief Finds the first relation 'r' greater than or equal to 'rel_start'
  /// in the range of all relations with label in [alpha, beta] such that the
  /// range is ordered in object major order.
  ///
  /// \remark In the literature this operation is known as \e rel_min_obj_maj
  ///
  pair_type lower_bound_object_major(label_id alpha, label_id beta,
                                     pair_type pair_start) const noexcept;

  /// \brief Returns the object of nth pair in the range of all relations
  /// with label 'fixed_label' and object >= 'object_start' such that the
  /// relation is in object major order.
  ///
  /// The nth object in the range
  /// (access(fixed_label, fixed_label, object_start, n)) sorted by objects.
  ///
  /// \pre n > 0
  ///
  /// \remark In the literature this operation is known as \e obj_sel1
  ///
  object_id object_select(label_id fixed_label, object_id object_start,
                          size_type nth) const noexcept;

  /// \brief Count the number of labels in the range lab_acc(alpha, beta, x, y).
  ///
  /// \remark In the literature this operation is known as \e lab_num
  ///
  size_type count_distinct_labels(object_id x, object_id y, label_id alpha,
                                  label_id beta) const noexcept;

  /// \brief Returns the number of different objects.
  ///
  size_type num_objects() const noexcept;

  /// \brief Returns the number of pairs in the binary relation.
  ///
  size_type size() const noexcept;

private:
  using index_type = wavelet_tree::index_type;

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

inline auto binary_relation::num_objects() const noexcept -> size_type {
  return m_bitmap.length() - size();
}

inline auto binary_relation::size() const noexcept -> size_type {
  return m_wtree.size();
};

} // end namespace brwt

#endif // BRWT_BINARY_RELATION_H
