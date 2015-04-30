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
 * \file DomainInfoMixedInteger.h
 *
 * Defines the scolib::DomainInfoMixedInteger class.
 */

#ifndef scolib_DomainInfoMixedInteger_h
#define scolib_DomainInfoMixedInteger_h

#include <acro_config.h>
#include <utilib/NumArray.h>
#include <scolib/DomainInfoBase.h>

namespace scolib {

///
class DomainInfoMixedInteger : public DomainInfoBase
{
public:

  ///
  DomainInfoMixedInteger() : epsa_flag(false) {}

  ///
  virtual ~DomainInfoMixedInteger() {}

  ///
  bool epsa_flag;

  ///
  utilib::NumArray<double> scale;

  ///
  void copy(const DomainInfoMixedInteger& info)
	{
	epsa_flag = info.epsa_flag;
	scale << info.scale;
	}

  ///
  void write(std::ostream& os) const
	{os << "EPSA Flag: " << epsa_flag << "  Scale: " << scale << std::endl;}

  ///
  void read(std::istream& /*is*/) 
	{}

  ///
  void write(utilib::PackBuffer& os) const
	{os << epsa_flag << scale;}

  ///
  void read(utilib::UnPackBuffer& is) 
	{is >> epsa_flag >> scale;}

};

}

#endif
