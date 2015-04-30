/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

/**
 * \file AppResponseLValues.h
 *
 * Defines the colin::AppResponseLValues class.
 */

#ifndef colin_AppResponseLValues_h
#define colin_AppResponseLValues_h

#include <acro_config.h>
#include <colin/AppResponseValues.h>

namespace colin
{

template <class TypesT>
class AppResponse;


///
/// A subclass of colin::AppResponseValues that contains pointers
/// to values and responses (which may not be owned by this class).
///
template <class TypesT, class LTypesT>
class AppResponseLValues: public AppResponseValues<TypesT>
{
public:

   /// Constructor.
   AppResponseLValues() : response_owned(false), response(0) {}

   /// Destructor.
   ~AppResponseLValues()
   {
      if (response && response_owned)
         utilib::CachedAllocator< AppResponse<LTypesT> > :: deallocate(response);
   }

   /// The values data.
   AppResponseValues<LTypesT>* values;

   /// If true, then this class owns this data.
   bool response_owned;

   /// The response data.
   AppResponse<LTypesT>* response;

   /// Initialize with colin::AppResponseInfo data.
   void init(AppResponseInfo* info)
{map_AppResponseValues(values, this, info); values->init(info);}

   /// Deallocator used with cached objects.
   void deallocate_derived()
   { utilib::CachedAllocator< AppResponseLValues<TypesT, LTypesT> >::deallocate_derived(this); }

};

} // namespace colin


#endif
