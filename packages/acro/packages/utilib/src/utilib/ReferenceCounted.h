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

#ifndef utilib_ReferenceCounted_h
#define utilib_ReferenceCounted_h

namespace utilib {

/** This template defines a "bare-bones" reference-counted object.  It
 *  offers very little functionality, but at the same time makes NO
 *  requirements on the contained object, apart from the ability to
 *  default-construct the contained object.
 *
 *  This class differs from the utilib::SmartPtr and utilib::SmartHandle
 *  classes in that the contained data is guaranteed to exist (there is
 *  no such thing as an "empty" ReferenceCounted object), and the
 *  contained class does not need to support operator<() or
 *  operator==().
 *
 *  utilib::ReferenceCounted may be thought of as a "poor-man's"
 *  boost::shared_ptr, and is included so that core utilib classes may
 *  maintain the majority of their functionality without introducing a
 *  strict dependence on Boost.  A future enhancement may be to switch
 *  ReferenceCounted to be a typedef of the boost::shared_ptr based on
 *  the UTILIB_HAVE_BOOST configuration define.
 */
template<typename TYPE>
class ReferenceCounted
{
private:
   struct Holder 
   {
      Holder()
         : data(), refCount(1)
      {}

      TYPE  data;
      size_t  refCount;
   };

   Holder* ref;

public:
   ReferenceCounted()
      : ref(new Holder)
   {}

   ReferenceCounted(const ReferenceCounted& rhs)
      : ref(rhs.ref)
   { ++ref->refCount; }

   ~ReferenceCounted()
   {
      if ( --ref->refCount == 0 )
         delete ref;
   }

   ReferenceCounted& operator=(const ReferenceCounted& rhs)
   { 
      if ( ref == rhs.ref )
         return *this;
      if ( --ref->refCount == 0 )
         delete ref;

      ref = rhs.ref;
      ++(ref->refCount); 
      return *this;
   }

   size_t use_count() const
   { return ref->refCount; }

   /// True if this object referecnces the same adata as the RHS
   bool operator==(const ReferenceCounted<TYPE> &rhs) const
   { return ref == rhs.ref; }

   /** NB: operator*() returns a non-const reference for the same reason
    *  boost::shared_ptr does:
    *
    *    "Shallow copy pointers, including raw pointers, typically don't
    *    propagate constness. It makes little sense for them to do so,
    *    as you can always obtain a non-const pointer from a const one
    *    and then proceed to modify the object through it.shared_ptr is
    *    'as close to raw pointers as possible but no closer'."
    */
   TYPE& operator*() const
   { return ref->data; }

   /** NB: operator->() returns a non-const pointer for the same reason
    *  boost::shared_ptr does:
    *
    *    "Shallow copy pointers, including raw pointers, typically don't
    *    propagate constness. It makes little sense for them to do so,
    *    as you can always obtain a non-const pointer from a const one
    *    and then proceed to modify the object through it.shared_ptr is
    *    'as close to raw pointers as possible but no closer'."
    */
   TYPE* operator->() const
   { return &ref->data; }
};


} // namespace utilib

#endif // utilib_ReferenceCounted_h
