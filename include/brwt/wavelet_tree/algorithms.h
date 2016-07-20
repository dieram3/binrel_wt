#ifndef BRWT_WAVELET_TREE_ALGORITHMS_H
#define BRWT_WAVELET_TREE_ALGORITHMS_H

#include <brwt/common_types.h> // symbol_id, size_type, index_type
#include <brwt/index_range.h>  // index_range
#include <utility>             // pair

namespace brwt {

class wavelet_tree;

/// \brief Counts the number of occurences of the given symbol in
/// <tt>S[0, pos]</tt>.
///
size_type inclusive_rank(const wavelet_tree& wt, symbol_id symbol,
                         index_type pos) noexcept;

/// \brief Counts the number of occurences of the given symbol in
/// <tt>S[0, pos)</tt>.
///
size_type exclusive_rank(const wavelet_tree& wt, symbol_id symbol,
                         index_type pos) noexcept;

/// \brief Counts the number of symbols in <tt>S[0, pos]</tt> such that the
/// symbol id is less than or equal to <tt>cond.max_value</tt>
///
size_type inclusive_rank(const wavelet_tree& wt, less_equal<symbol_id> cond,
                         index_type pos) noexcept;

/// \brief Counts the number of symbols in <tt>S[0, pos)</tt> such that the
/// symbol-id is less than or equal to <tt>cond.max_value</tt>.
///
size_type exclusive_rank(const wavelet_tree& wt, less_equal<symbol_id> cond,
                         index_type pos) noexcept;

/// \brief Counts the number of distinct symbols in the specified range.
///
size_type count_distinct_symbols(const wavelet_tree& wt,
                                 index_range range) noexcept;

/// \brief Counts the number of distinct symbols in the specified range, with
/// symbol-id less than or equal to <tt>cond.max_value</tt>.
///
size_type count_distinct_symbols(const wavelet_tree& wt, index_range range,
                                 less_equal<symbol_id> cond) noexcept;

/// \brief Counts the number of distinct symbols in the specified range, with
/// symbol-id greater than or equal to <tt>cond.min_value</tt>.
///
size_type count_distinct_symbols(const wavelet_tree& wt, index_range range,
                                 greater_equal<symbol_id> cond) noexcept;

/// \brief Counts the number of distinct symbols in the specified range, with
/// symbol-id not less than cond.min_value and not greater than
/// <tt>cond.max_value</tt>.
///
size_type count_distinct_symbols(const wavelet_tree& wt, index_range range,
                                 between<symbol_id> cond) noexcept;

/// \brief Returns the element that would occur in the nth position of the given
/// range if it was sorted by symbol-id.
///
/// \returns A pair containing the \e nth symbol along with its position in S
/// (the original sequence).
///
/// \par Time Complexity
/// <tt>O(log(sigma))</tt>, where \c sigma refers to the alphabet size.
///
std::pair<symbol_id, index_type>
nth_element(const wavelet_tree& wt, index_range range, size_type nth) noexcept;

} // end namespace brwt

#endif // BRWT_WAVELET_TREE_ALGORITHMS_H
