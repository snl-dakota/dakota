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
 * \file DomainOpsBase.h
 *
 * Defines the scolib::DomainOpsBase class.
 */

#ifndef scolib_EAgenericDomainOpsBase_h
#define scolib_EAgenericDomainOpsBase_h

#include <acro_config.h>
#include <utilib/AnyRNG.h>
#include <utilib/ParameterSet.h>

namespace scolib {


///
/// A base class that can be used to define the OpsClass needed for a 
/// EAgeneric object.
///
template <class DomainT, class InfoT>
class DomainOpsBase : public utilib::ParameterSet
{
public:

  ///
  DomainOpsBase() {}

  ///
  virtual void reset() {}

  ///
   virtual void write(std::ostream& /*os*/) const {}

  ///
   virtual void randomize(DomainT& /*point*/, InfoT& /*info*/) {}

  ///
   virtual void initialize_point(DomainT& /*point*/, InfoT& /*info*/) {}

  ///
   virtual void read_point(std::istream& /*istr*/, DomainT& /*point*/) {}

  ///
   virtual int apply_xover(DomainT& /*parent1*/, InfoT& /*info1*/, 
			  DomainT& /*parent2*/, InfoT& /*info2*/,
			  DomainT& /*child*/,   InfoT& /*info_c*/)
                        {return 3;}

  ///
   virtual bool apply_mutation(DomainT& /*point*/, InfoT& /*info*/,
					int /*parent_ndx*/)
                        {return false;}

  ///
  virtual void set_rng(utilib::AnyRNG& /*rng_*/) {}

};

}

#endif
