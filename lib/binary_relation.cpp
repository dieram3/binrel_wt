#include <brwt/binary_relation.h>

#include <brwt/bit_hacks.h>   // used_bits
#include <brwt/index_range.h> // index_range
#include <algorithm>          // max, for_each
#include <cassert>            // assert
#include <cstddef>            // size_t
#include <iterator>           // begin, end
#include <numeric>            // partial_sum
#include <tuple>              // tie
#include <type_traits>        // is_enum, underlying_type_t
#include <utility>            // pair, make_pair, move
#include <vector>             // vector

namespace brwt {

using std::size_t;
using node_proxy = wavelet_tree::node_proxy;

using object_id = binary_relation::object_id;
using label_id = binary_relation::label_id;
using pair_type = binary_relation::pair_type;

// ==========================================
// Generic helpers
// ==========================================

// TODO(Diego): Consider to put exclusive_scan in a private header. It is also
// used in the wavelet tree implementation.
template <typename InputIt, typename OutputIt, typename T>
static void exclusive_scan(InputIt first, const InputIt last, OutputIt d_first,
                           T init) {
  for (; first != last; ++first) {
    *d_first++ = std::exchange(init, init + (*first));
  }
}

template <typename InputRange, typename UnaryFunction>
static UnaryFunction for_each(const InputRange& range, UnaryFunction f) {
  return std::for_each(std::begin(range), std::end(range), f);
}

template <typename T>
static constexpr auto to_underlying_type(const T value) noexcept {
  static_assert(std::is_enum<T>::value, "");
  return static_cast<std::underlying_type_t<T>>(value);
}

// ==========================================
// binary_relation implementation
// ==========================================

static constexpr auto as_symbol(const label_id label) noexcept {
  return static_cast<symbol_id>(label);
}

static constexpr object_id prev(const object_id x) noexcept {
  assert(to_underlying_type(x) != 0);
  return static_cast<object_id>(to_underlying_type(x) - 1);
}

static constexpr label_id prev(const label_id alpha) noexcept {
  assert(to_underlying_type(alpha) != 0);
  return static_cast<label_id>(to_underlying_type(alpha) - 1);
}

[[deprecated("when there is no pair with object_id=x, this function does not "
             "work as expected")]] auto
binary_relation::map(const object_id x) const noexcept -> index_type {
  // Note that in some cases upper_bound(x) == 0 so this function would return
  // -1 (an invalid index).
  return upper_bound(x) - 1;
}

[[deprecated(
    "the function 'map' will be removed, so this function would be "
    "unclear. Prefer get_associated_object() member function instead")]] auto
binary_relation::unmap(const index_type wt_pos) const noexcept -> object_id {
  return get_associated_object(wt_pos);
}

/// Returns the position of the first element in the wavelet tree such that its
/// associated object has value >= x.
/// In the stored wavelet tree, the range [lower_bound(x), wt.size()) contains
/// the labels of all the 'pairs' with object >= x (where 'pairs' refer to the
/// pairs of the original sequence).
/// If no such element exists, return m_wtree.size().
///
auto binary_relation::lower_bound(const object_id x) const noexcept
    -> index_type {
  if (x == 0) {
    return 0;
  }
  const auto flag_pos = m_bitmap.select_1(x);
  return m_bitmap.rank_0(flag_pos);
}

/// Returns the position of the first element in the wavelet tree such that its
/// associated object has value > x.
/// In the stored wavelet tree, the range [0, upper_bound(x)) contains the
/// labels of all the 'pairs' with object <= x (where 'pairs' refer to the pairs
/// of the original sequence).
/// If no such element exists, return m_wtree.size().
///
auto binary_relation::upper_bound(const object_id x) const noexcept
    -> index_type {
  const auto flag_pos = m_bitmap.select_1(x + 1);
  return m_bitmap.rank_0(flag_pos);
}

/// Returns the range of elements with: (object >= x && object <= y)
///
auto binary_relation::make_mapped_range(const object_id x,
                                        const object_id y) const noexcept {
  // TODO(Diego): Consider renaming this member function.
  assert(x <= y);
  return index_range{lower_bound(x), upper_bound(y)};
}

/// Returns the associated object of the element in the wavelet tree at
/// position \p wt_pos.
///
auto binary_relation::get_associated_object(const index_type wt_pos) const
    noexcept -> object_id {
  assert(wt_pos >= 0 && wt_pos < m_wtree.size());

  const auto bit_pos = m_bitmap.select_0(wt_pos + 1);
  return static_cast<object_id>(m_bitmap.rank_1(bit_pos));
}

namespace pairs_constructor_detail {
using std::vector;

static auto count_objects_frequency(const vector<pair_type>& pairs,
                                    const object_id max_object) {
  // TODO(Diego): Consider to replace the histogram with a compressed vector.
  const auto num_objects = static_cast<size_t>(max_object) + 1;
  std::vector<size_type> frequency(num_objects);
  for_each(pairs, [&](const pair_type& p) { ++frequency[p.object]; });
  return frequency;
}

static bitmap make_bitmap(const vector<size_type>& objects_frequency,
                          const size_type num_pairs) {
  auto bit_seq = [&] {
    const auto num_objects = static_cast<size_type>(objects_frequency.size());
    return bit_vector(/*count=*/num_pairs + num_objects);
  }();

  size_type acc_count = 0;
  for_each(objects_frequency, [&](const size_type count) {
    acc_count += count;
    bit_seq.set(acc_count, true);
    acc_count += 1;
  });
  return bitmap(std::move(bit_seq));
}

template <typename ForwardRange, typename T>
static void inplace_exclusive_scan(ForwardRange& range, const T init) {
  exclusive_scan(std::begin(range), std::end(range), std::begin(range), init);
}

static wavelet_tree make_wavelet_tree(const vector<pair_type>& pairs,
                                      vector<size_type> objects_frequency,
                                      const label_id max_label) {
  int_vector seq(/*count=*/static_cast<size_type>(pairs.size()),
                 /*bpe=*/used_bits(types::word_type{max_label}));

  // The exclusive_scan and the for_each are an inline counting sort.
  inplace_exclusive_scan(objects_frequency, size_type{0});
  for_each(pairs, [&](const pair_type& p) {
    const auto next_pos = objects_frequency[p.object]++;
    seq[next_pos] = p.label;
  });

  assert(objects_frequency.back() == seq.size());

  return wavelet_tree(std::move(seq));
}
} // end namespace pairs_constructor_detail

binary_relation::binary_relation(const std::vector<pair_type>& pairs) {
  object_id max_object{};
  label_id max_label{};
  for_each(pairs, [&](const pair_type& pair) {
    max_object = std::max(max_object, pair.object);
    max_label = std::max(max_label, pair.label);
  });

  namespace detail = pairs_constructor_detail;
  const auto num_pairs = static_cast<size_type>(pairs.size());
  const auto objects_frequency =
      detail::count_objects_frequency(pairs, max_object);

  m_bitmap = detail::make_bitmap(objects_frequency, num_pairs);
  m_wtree =
      detail::make_wavelet_tree(pairs, std::move(objects_frequency), max_label);
}

auto binary_relation::rank(object_id max_object, label_id max_label) const
    noexcept -> size_type {
  return exclusive_rank(m_wtree, less_equal<symbol_id>{as_symbol(max_label)},
                        upper_bound(max_object));
}

auto binary_relation::rank(object_id min_object, object_id max_object,
                           label_id max_label) const noexcept -> size_type {
  if (min_object == 0) {
    return rank(max_object, max_label);
  }
  return rank(max_object, max_label) - rank(prev(min_object), max_label);
}

auto binary_relation::nth_element(const object_id x, const object_id y,
                                  const label_id alpha, size_type nth,
                                  label_major_order_t /*unused*/) const noexcept
    -> optional<pair_type> {
  assert(x <= y);
  assert(nth > 0);

  if (alpha > 0) {
    nth += rank(x, y, prev(alpha));
  }

  const auto range = make_mapped_range(x, y);
  if (range.size() < nth) {
    return nullopt;
  }

  symbol_id symbol{};
  index_type wt_pos{};
  std::tie(symbol, wt_pos) = brwt::nth_element(m_wtree, range, nth);

  return pair_type{get_associated_object(wt_pos), label_id(symbol)};
}

// auto binary_relation::nth_element(const object_id x, const label_id alpha,
//                                   const label_id beta, const size_type nth,
//                                   object_major_order_t /*unused*/) const
//     noexcept -> optional<pair_type> {
//   // TODO(Diego): Implement it.
// }

auto binary_relation::object_select(const label_id fixed_label,
                                    const object_id object_start,
                                    const size_type nth) const noexcept
    -> object_id {

  const auto abs_nth = nth + exclusive_rank(m_wtree, as_symbol(fixed_label),
                                            lower_bound(object_start));

  const auto wt_pos = m_wtree.select(as_symbol(fixed_label), abs_nth);
  assert(wt_pos != -1 && "The element was supposed to exist");

  // TODO(Diego): Design decision. Determine what to do if wt_pos == -1, that
  // is, if the searched element does not exist. Note that returning -1 is not
  // possible as the return type is an object id, not an index.

  return get_associated_object(wt_pos);
}

auto binary_relation::count_distinct_labels(const object_id x,
                                            const object_id y,
                                            const label_id alpha,
                                            const label_id beta) const noexcept
    -> size_type {
  const auto range = make_mapped_range(x, y);
  const auto cond = between<symbol_id>{as_symbol(alpha), as_symbol(beta)};
  return count_distinct_symbols(m_wtree, range, cond);
}

} // end namespace brwt
