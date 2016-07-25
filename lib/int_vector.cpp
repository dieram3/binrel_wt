#include <brwt/int_vector.h> // int_vector

#include <brwt/bit_hacks.h> // lsb_mask, used_bits
#include <algorithm>        // for_each, max, copy
#include <cassert>          // assert
#include <iterator>         // begin, end
#include <limits>           // numeric_limits
#include <stdexcept>        // domain_error

namespace brwt {

template <typename InputIt>
static int needed_bits(InputIt first, InputIt last) {
  int ans = 0;
  std::for_each(first, last, [&ans](const auto value) {
    ans = std::max(ans, used_bits(value));
  });
  return ans;
}

template <typename InputRange>
static int needed_bits(const InputRange& range) {
  return needed_bits(std::begin(range), std::end(range));
}

// ==========================================
// int_vector implementation
// ==========================================

int_vector::int_vector(const size_type count, const int bpe)
    : bit_seq{}, num_elems{count}, bits_per_element{bpe} {
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

void int_vector::set_value(const size_type pos,
                           const value_type value) noexcept {
  assert(pos >= 0 && pos < num_elems && "Out of range");
  assert(value <= lsb_mask<value_type>(static_cast<int>(bits_per_element)));

  bit_seq.set_chunk(pos * bits_per_element, bits_per_element, value);
}

} // end namespace brwt
