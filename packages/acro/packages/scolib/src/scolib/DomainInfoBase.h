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
 * \file DomainInfoBase.h
 *
 * Defines the scolib::DomainInfoBase class.
 */

#ifndef scolib_DomainInfoBase_h
#define scolib_DomainInfoBase_h

#include <acro_config.h>
#include <utilib/std_headers.h>
#include <utilib/PackBuf.h>
#include <utilib/PackObject.h>

namespace scolib {

///
class DomainInfoBase : public utilib::PackObject
{
public:

  ///
  virtual ~DomainInfoBase() {}

  ///
  void copy(const DomainInfoBase& ) {}

  ///
   virtual void write(std::ostream& /*os*/) const {}

  ///
   virtual void read(std::istream& /*is*/) {}

  ///
   virtual void write(utilib::PackBuffer& /*os*/) const {}

  ///
   virtual void read(utilib::UnPackBuffer& /*is*/) {}

};

}

#endif
