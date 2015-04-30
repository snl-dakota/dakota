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
// BitArray.cpp
//
// All internal arrays are indexed 0..n-1
//

#include <utilib/std_headers.h>
#include <utilib/BitArray.h>
#include <utilib/TypeManager.h>

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif

namespace utilib {
namespace {
int cast_bitArray_to_vector(const Any& from, Any& to)
{
   std::vector<bool> &ans = to.set<std::vector<bool> >();
   const BitArray &src = from.expose<BitArray>();

   size_t i = src.size();
   ans.resize(i);
   for( ; i > 0; --i, ans[i] = src(i) );
   return OK;
}

int cast_vector_to_bitArray(const Any& from, Any& to)
{
   BitArray &ans = to.set<BitArray>();
   const std::vector<bool> &src = from.expose<std::vector<bool> >();

   size_t i = src.size();
   ans.resize(i);
   for( ; i > 0; --i, src[i] ? ans.set(i) : ans.reset(i) );
   return OK;
}

} // namespace utilib::(local)

const volatile bool BitArray::registrations_complete = 
   BitArray::register_aux_functions();

bool BitArray::register_aux_functions()
{
   Serializer().register_serializer<BitArray>
      ( "utilib::BitArray", BitArray::serializer );

   TypeManager()->register_lexical_cast
      ( typeid(BitArray), typeid(std::vector<bool>), 
        &cast_bitArray_to_vector );
   TypeManager()->register_lexical_cast
      ( typeid(std::vector<bool>), typeid(BitArray), 
        &cast_vector_to_bitArray );

   return true;
}


size_type BitArray::nbits() const
{
  size_type counter = 0;

  // Do all the full words of data

  size_type numFullWords = Len >> index_shift;
  size_type i = 0;
  for (; i<numFullWords; i++)
    count_bits(counter,Data[i]);

  // Check the last bitword, if any

  size_type leftover = static_cast<size_type>(Len & index_mask);
  if (leftover > 0)
    {
      workingBitData mask = (1 << leftover) - 1;
      count_bits(counter,Data[i] & mask);
    }
  
  return counter;
}


void BitArray::set()
{ 
  size_type nwords = alloc_size(Len);
  for(size_type i=0; i<nwords; i++)
    Data[i] = ~0;
}


void BitArray::reset()
{
  size_type nwords = alloc_size(Len);
  for(size_type i=0; i<nwords; i++)
    Data[i] = 0;
}


void BitArray::flip()
{
  size_type nwords = alloc_size(Len);
  for(size_type i=0; i<nwords; i++)
    Data[i] = ~Data[i];
}


void bitwise_or(BitArray& a1, BitArray& a2, BitArray& result)
{
  if ((a1.size() != a2.size()) || (a1.size() != result.size()    ))
     EXCEPTION_MNGR(runtime_error, "bitwise_or : inconsistent array lengths: a1=" << a1.size() << " a2=" << a2.size() << " result=" << result.size());
  size_type nwords = a1.alloc_size(a1.size());
  for (size_type i=0; i<nwords; i++)
    result.Data[i] = a1.Data[i] | a2.Data[i];
}


void bitwise_xor(BitArray& a1, BitArray& a2, BitArray& result)
{
  if ((a1.size() != a2.size()) || (a1.size() != result.size()    ))
     EXCEPTION_MNGR(runtime_error, "bitwise_xor : inconsistent array lengths: a1=" << a1.size() << " a2=" << a2.size() << " result=" << result.size());
  size_type nwords = a1.alloc_size(a1.size());
  for (size_type i=0; i<nwords; i++)
    result.Data[i] = a1.Data[i] ^ a2.Data[i];
}


void bitwise_and(BitArray& a1, BitArray& a2, BitArray& result)
{
  if ((a1.size() != a2.size()) || (a1.size() != result.size()    ))
     EXCEPTION_MNGR(runtime_error, "bitwise_and : inconsistent array lengths: a1=" << a1.size() << " a2=" << a2.size() << " result=" << result.size());
  size_type nwords = a1.alloc_size(a1.size());
  for (size_type i=0; i<nwords; i++)
    result.Data[i] = a1.Data[i] & a2.Data[i];
}


// return true (1) if the two BitArrays both have a 1 in at least one
// bit.  Equivalent to a bitwise AND and then comparing nbits() on the result
// to 0, but much more efficient (e.g. can return as soon as find agreement)

int BitArray::shared_one(BitArray& other) const
{
size_type min_length = other.size();
if (size() < min_length)
  min_length = size();

  // Do all the full words of data

  size_type numFullWords = Len >> index_shift;
  size_type i = 0;
  for (; i<numFullWords; i++)
    if (Data[i] & other.Data[i])
      return 1;

  // Check the last word, if any

  size_t leftover = (Len & index_mask);
  if (leftover > 0)
    {
      workingBitData mask = (1 << leftover) - 1;
      if (Data[i] & other.Data[i] & mask)
	return 1;
    }
  
  return 0;
}


int BitArray::serializer( SerialObject::elementList_t& serial, 
                          Any& data, bool serialize )
{
   // The following is a trick to cast the incoming Any into the base
   // class (needed until Anys support polymorphism). 
   Any tmp;
   tmp.set<BitArrayBase<0,int,BitArray> >(data.expose<BitArray>(), true);
   // NB: directly calling the base class serializer ONLY works
   // because this derived class does not add any additional data.
   // That said, doing this saves a level of indirection in the
   // actual serial stream.
   return BitArrayBase<0,int,BitArray>::serializer(serial, tmp, serialize); 
}


} // namespace utilib
