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

/**
 * \file RefCount.h
 *
 * \deprecated
 */

#ifndef utilib_RefCount_h
#define utilib_RefCount_h

namespace utilib {

#ifndef _ENUMDATAOWNED_
#define _ENUMDATAOWNED_
/**
 * Ownership categories for objects with reference counts.
 */
enum EnumDataOwned
{
  DataNotOwned=0,       /**< Data owned by some other object */
  DataOwned=1,          /**< Memory allocated by object itself */
  AcquireOwnership=1,   /**< Synonym for DataOwned */
  AssumeOwnership=2     /**< We own it now, but it comes from elsewhere */
                        /**< Once the object has been made this is      */
                        /**< identical to DataOwned                     */
};
#endif


/// An object used to maintain reference counts for shared data.
template <class Type>
class RefCount
{
public:

  /// Constructor
  RefCount(Type* type_, EnumDataOwned own_=DataNotOwned)
        {Data = type_; own = own_; nref=1;}

  /// Destructor
  ~RefCount()
        {decrement();}

  /// Set the data in this class
  void set_data(Type* data_, EnumDataOwned own_=DataNotOwned)
        {
        if (own)
           delete Data;
        Data = data_;
        own = own_;
        }

  /// Return a pointer to the data
  Type* data()
        {return Data;}

  /// Return a const pointer to the data
  const Type* data() const
        {return Data;}

  /// Increment the reference counter
  void increment()
        {nref++;}

  /// Returns true if there are one or more references to the data
  operator bool()
        {return (nref > 0);}

  /// Decrement the reference counter.
  /// If this is decremented to zero, then the data is deleted if it is
  /// owned by this class.
  bool decrement()
        {
        nref--;
        if (own && (nref == 0))
           delete Data;
        return (nref == 0);
        }

protected:

  /// The data managed by this class.
  Type* Data;

  /// The ownership of this data.
  EnumDataOwned own;

  /// The reference counter for this data.
  int nref;
};


} // namespace utilib

#endif
