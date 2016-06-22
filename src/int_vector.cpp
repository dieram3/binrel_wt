#include <brwt/int_vector.h> // int_vector

#include <brwt/bit_hacks.h> // lsb_mask
#include <cassert>          // assert
#include <limits>           // numeric_limits
#include <stdexcept>        // domain_error

using brwt::int_vector;

// ==========================================
// int_vector implementation
// ==========================================

int_vector::int_vector(const size_type count, const int bpe)
    : bit_seq{}, num_elems{count}, bits_per_element{bpe} {
  assert(count >= 0);
  assert(bpe >= 0);

  constexpr auto bits_per_block = std::numeric_limits<value_type>::digits;
  if (bits_per_element >= bits_per_block)
    throw std::domain_error("int_vector: Too many bits per element");

  bit_seq = bit_vector(num_elems * bits_per_element);
}

void int_vector::set_value(const size_type pos, const value_type value) {
  assert(pos >= 0 && pos < num_elems && "Out of range");

  if (value > lsb_mask<value_type>(static_cast<int>(bits_per_element))) {
    throw std::domain_error("int_vector: Value too large to be stored");
  }

  bit_seq.set_chunk(pos * bits_per_element, bits_per_element, value);
}
