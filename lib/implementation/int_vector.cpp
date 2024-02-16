module;

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <stdexcept>

module brwt.int_vector;

import brwt.bit_ops;
import brwt.bit_vector;

namespace brwt {

namespace {

template <typename InputIt>
int needed_bits(const InputIt first, const InputIt last) {
  if (first == last) {
    return 0;
  }
  const auto max = *std::max_element(first, last);
  return max == 0 ? 1 : used_bits(max);
}

template <typename InputRange>
int needed_bits(const InputRange& range) {
  return needed_bits(std::begin(range), std::end(range));
}

} // namespace

auto int_vector::set_value(const size_type pos, const value_type value) noexcept
    -> void {
  assert(pos >= 0 && pos < num_elems && "Out of range");
  assert(value <= lsb_mask<value_type>(static_cast<int>(bits_per_element)));

  bit_seq.set_chunk(pos * bits_per_element, bits_per_element, value);
}

auto int_vector::index_of(const_iterator pos) const noexcept -> size_type {
  assert(pos >= cbegin() && pos <= cend());

  return pos - cbegin();
}

auto int_vector::non_const(const_iterator pos) noexcept -> iterator {
  assert(pos >= begin() && pos <= end());

  return begin() + index_of(pos);
}

int_vector::int_vector(const size_type count, const int bpe)
    : num_elems{count}, bits_per_element{bpe} {
  assert(count >= 0);
  assert(bpe >= 0);

  constexpr auto bits_per_block = std::numeric_limits<value_type>::digits;
  if (bits_per_element >= bits_per_block) {
    throw std::domain_error("int_vector: Too many bits per element");
  }

  bit_seq = bit_vector(num_elems * bits_per_element);
}

int_vector::int_vector(std::initializer_list<value_type> ilist)
    : int_vector(static_cast<size_type>(ilist.size()), needed_bits(ilist)) {

  std::copy(ilist.begin(), ilist.end(), begin());
}

auto int_vector::erase(const_iterator pos) noexcept -> iterator {
  assert(pos >= begin() && pos < end());
  return erase(pos, pos + 1);
}

auto int_vector::erase(const_iterator first, const_iterator last) noexcept
    -> iterator {
  assert(first >= begin() && first <= last && last <= end());

  // Note that num_elems must be updated at the end because of cend() usage.
  const auto new_end = std::copy(last, cend(), non_const(first));
  num_elems = index_of(new_end);

  return non_const(first);
}

} // end namespace brwt
