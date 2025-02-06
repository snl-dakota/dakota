/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_BIT_UTILS_H
#define DAKOTA_BIT_UTILS_H

#include "dakota_data_types.hpp"

namespace Dakota {

/// Bit operations are taken from the Stanford Bithack webpage
/// http://www.graphics.stanford.edu/~seander/bithacks.html

/// Get the bit at position 'j'
template <typename T>
inline T bitget(T v, int j)
{
  return (v >> j) & T(1);
}

/// Count consecutive trailing zero bits
inline unsigned count_consecutive_trailing_zero_bits(UInt32 v)
{
  unsigned c = 32;
  v &= -signed(v);
  if (v) c--;
  if (v & 0x0000FFFF) c -= 16;
  if (v & 0x00FF00FF) c -= 8;
  if (v & 0x0F0F0F0F) c -= 4;
  if (v & 0x33333333) c -= 2;
  if (v & 0x55555555) c -= 1;
  return c;
}

/// Reverse bits of unsigned 32 bit integer
inline UInt32 bitreverse(UInt32 k)
{
  UInt32 v = k;
  v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);  // swap odd and even bits
  v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);  // swap consecutive pairs
  v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);  // swap nibbles ...
  v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);  // swap bytes
  v = ( v >> 16             ) | ( v               << 16); // swap 2-byte long pairs
  return v;
}

/// Reverse bits of unsigned 64 bit integer
inline UInt64 bitreverse(UInt64 v)
{
  return (UInt64(bitreverse(UInt32(v))) << 32)
    | UInt64(bitreverse(UInt32(v >> 32)));
}

/// Reverse bits of arbitrary unsigned integer
template<typename T>
inline T bitreverse(T v, int s)
{
  T r = 0;
  for ( size_t j = 0; j < s; j++ )
  {
    r <<= 1;
    r |= v & 1;
    v >>= 1;
  }
  return r;
}

/// Convert binary to Gray code order
inline UInt64 binary2gray(UInt64 v)
{
  return v ^ (v >> 1);
}

/// Check if given integer is a power of 2
inline bool ispow2(unsigned v)
{
  return v && !(v & (v - 1));
}

/// Matrix matrix multiplication in base 2
template<typename UnsignedArrayType>
UnsignedArrayType matmul_base2(
  UnsignedArrayType A,
  UnsignedArrayType B
)
{
  auto t = A.length();
  auto m = B.length();

  UnsignedArrayType result(m);
  for ( size_t k = 0; k < m; k++ )
  {
    for ( size_t i = 0; i < t; i ++ )
    {
      result(k) ^= bitget(B(k), i) * A(i);
    }
  }
  return result;
}

} // namespace Dakota

#endif // DAKOTA_BIT_UTILS_H