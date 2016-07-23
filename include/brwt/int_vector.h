#ifndef BRWT_INT_VECTOR_H
#define BRWT_INT_VECTOR_H

#include <brwt/bit_vector.h> // bit_vector
#include <cassert>           // assert

namespace brwt {

class int_vector {
public:
  using value_type = bit_vector::block_type;
  using size_type = bit_vector::size_type;

  /// \brief Class to provide an l-value reference to a particular element from
  /// the array.
  class reference {
  private:
    reference(int_vector& vec, size_type pos) noexcept
        : vector{vec}, elem_pos{pos} {}
    reference(const reference&) = default;

  public:
    reference& operator=(const value_type value) noexcept {
      vector.set_value(elem_pos, value);
      return *this;
    }
    reference& operator=(const reference& other) noexcept {
      vector.set_value(elem_pos, other);
      return *this;
    }
    operator value_type() const noexcept {
      return vector.get_value(elem_pos);
    }

  private:
    int_vector& vector;
    size_type elem_pos;
    friend int_vector;
  };

  friend reference;

public:
  /// \brief Constructs an empty vector.
  ///
  /// \par TIme complexity
  /// Constant.
  ///
  int_vector() = default;

  /// \brief Constructs a fixed-size vector with the given size requirements.
  ///
  /// All elements of the vector are zeroed.
  ///
  /// \param count The number of elements to store.
  /// \param bpe The number of bits per element to use.
  ///
  /// \par Time complexity
  /// Linear in <tt>count * bpe</tt>.
  ///
  /// \throws std::domain_error if \p bits is greater than or equal to the
  /// number of bits of <tt>value_type</tt>.
  ///
  int_vector(size_type count, int bpe);

  /// \brief Returns a proxy to the element at the given position.
  ///
  /// \param pos The position of the element to access.
  ///
  /// \par Time complexity
  /// Constant.
  ///
  reference operator[](size_type pos) noexcept {
    return reference(*this, pos);
  }

  /// \brief Return the value of the element at a given position.
  ///
  /// \param pos The position of the element to access.
  ///
  /// \par Time complexity
  /// Constant.
  ///
  value_type operator[](size_type pos) const noexcept {
    return get_value(pos);
  }

  /// \brief Returns the number of elements of the array.
  ///
  /// \par Time complexity
  /// Constant.
  ///
  size_type size() const noexcept {
    return num_elems;
  }

  /// \brief Returns the number of bits per element.
  ///
  /// \par Time complexity
  /// Constant.
  ///
  size_type get_bpe() const noexcept {
    return bits_per_element;
  }

  /// \brief Returns the number of allocated bytes.
  ///
  /// \par Time complexity
  /// Constant.
  ///
  size_type allocated_bytes() const noexcept {
    return bit_seq.allocated_bytes();
  }

private:
  value_type get_value(size_type pos) const noexcept;
  void set_value(size_type pos, value_type value) noexcept;

private:
  bit_vector bit_seq;
  size_type num_elems{};
  size_type bits_per_element{};
};

// ==========================================
// Inline definitions
// ==========================================

inline int_vector::value_type int_vector::get_value(const size_type pos) const
    noexcept {
  assert(pos >= 0 && pos < num_elems && "Out of range");
  return bit_seq.get_chunk(pos * bits_per_element, bits_per_element);
}

} // end namespace brwt

#endif // BRWT_INT_VECTOR_H
