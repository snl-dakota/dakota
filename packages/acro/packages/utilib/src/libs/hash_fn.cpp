/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

//
// hash_fn.cpp
//
// WEH: note that I have not tested the fidelity of these hash functions!
//

#include <utilib/std_headers.h>
#include <utilib/hash_fn.h>

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif

namespace utilib {

/* make a good hash function for any int size */
/* inspired by Aho, Sethi and Ullman, Compilers ..., p436 */
/* adapted from code developed in lpsolve3.0 */
#define HASH_1 sizeof(size_type)
#define HASH_2 (sizeof(size_type) * 6)
#define HASH_3 (((size_type)0xF0) << ((sizeof(size_type) - 1) * CHAR_BIT))



size_type hash_fn1(const CharString& str, size_type table_size)
{
if (table_size == 0)
   return 0;

size_type result = 0, tmp;
for (size_type i=0; (i<str.size()) && str[i]; i++) {
  result = (result << HASH_1) + str[i];
  if ((tmp = result & HASH_3) != 0) {
      /* if any of the most significant bits is on */
      result ^= tmp >> HASH_2; /* xor them in a less significant part */
      result ^= tmp; /* and reset the most significant bits to 0 */
      }
  }
return (result % table_size);
}


size_type hash_fn2(const CharString& str, size_type table_size)
{
if (table_size == 0)
   return 0;

size_type result = 0;
for (size_type i=0; (i<str.size()) && str[i]; i++)
  result = 131*result + str[i];

return (result % table_size);
}


//
// See "Performance in Practice of String Hashing Functions",
// M.V. Ramakrishna, J. Zobel, Dept CS, RMIT, Proc of 5th Intl
// Conf on Database Systems for Advanced Applications,
// April, 1997.
//
size_type hash_fn3(const CharString& str, size_type table_size)
{
if (table_size == 0)
   return 0;

size_type result = 0;
for (size_type i=0; (i<str.size()) && str[i]; i++)
  result = result ^ ((result << 5) + (result >> 2) + str[i]);

return (result % table_size);
}



//
// See "Performance in Practice of String Hashing Functions",
// M.V. Ramakrishna, J. Zobel, Dept CS, RMIT, Proc of 5th Intl
// Conf on Database Systems for Advanced Applications,
// April, 1997.
//
size_type hash_fn3(const string& str, size_type table_size)
{
if (table_size == 0)
   return 0;

size_type result = 0;
for (size_type i=0; i<str.size(); i++)
  result = result ^ ((result << 5) + (result >> 2) + str[i]);

return (result % table_size);
}




#define A 2.6397813781

size_type hash_fn1(const BasicArray<int>& vec, size_type table_size)
{
if (vec.size() == 0)
   return 0;

size_type result = table_size;

for (size_type i=0; i<vec.size(); i++)
  result = (size_type) floor( result * fmod( (fabs((double)vec[i])+1.0)*A, 1.0 ) ) + table_size;

return (result % table_size);
}


size_type hash_fn1(const BasicArray<double>& vec, size_type table_size)
{
if (vec.size() == 0)
   return 0;

size_type result = table_size;

for (size_type i=0; i<vec.size(); i++)
  result = (size_type) floor( result * fmod( (fabs((double)vec[i])+1.0)*A, 1.0 ) ) + table_size;

return (result % table_size);
}


size_type hash_fn2(const BasicArray<double>& vec, size_type table_size)
{
if (vec.size() == 0)
   return 0;

size_type result = table_size;

result = (size_type) floor( result * fmod( (fabs((double)vec[0])+1.0)*A, 1.0 ) ) + table_size;

for (size_type i=1; i<vec.size(); i++)
  result = result^((result >> 11) + (result << 5) + (size_type) floor( result * fmod( (fabs((double)vec[i])+1.0)*A, 1.0 )) );

return (result % table_size);
}


size_type hash_fn1(const std::vector<int>& vec, size_type table_size)
{
if (vec.size() == 0)
   return 0;

size_type result = table_size;

for (size_type i=0; i<vec.size(); i++)
  result = (size_type) floor( result * fmod( (fabs((double)vec[i])+1.0)*A, 1.0 ) ) + table_size;

return (result % table_size);
}


size_type hash_fn1(const std::vector<double>& vec, size_type table_size)
{
if (vec.size() == 0)
   return 0;

size_type result = table_size;

for (size_type i=0; i<vec.size(); i++)
  result = (size_type) floor( result * fmod( (fabs((double)vec[i])+1.0)*A, 1.0 ) ) + table_size;

return (result % table_size);
}


size_type hash_fn2(const std::vector<double>& vec, size_type table_size)
{
if (vec.size() == 0)
   return 0;

size_type result = table_size;

result = (size_type) floor( result * fmod( (fabs((double)vec[0])+1.0)*A, 1.0 ) ) + table_size;

for (size_type i=1; i<vec.size(); i++)
  result = result^((result >> 11) + (result << 5) + (size_type) floor( result * fmod( (fabs((double)vec[i])+1.0)*A, 1.0 )) );

return (result % table_size);
}


size_type hash_fn1(const int& vec, size_type table_size)
{
size_type result = table_size;
result = (size_type) floor( result * fmod( (fabs((double)vec)+1.0)*A, 1.0 ) ) + table_size;
return (result % table_size);
}


size_type hash_fn1(const double& vec, size_type table_size)
{
size_type result = table_size;
result = (size_type) floor( result * fmod( (fabs((double)vec)+1.0)*A, 1.0 ) ) + table_size;
return (result % table_size);
}




int utilib_num_primes = 28;

unsigned long utilib_prime_list[] =
        {
        53,         97,           193,         389,       769,
        1543,       3079,         6151,        12289,     24593,
        49157,      98317,        196613,      393241,    786433,
        1572869,    3145739,      6291469,     12582917,  25165843,
        50331653,   100663319,    201326611,   402653189, 805306457,
        1610612741, 3221225473ul, 4294967291ul
        };

} // namespace utilib

