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
 * \file EApoint.h
 *
 * Defines the scolib::EApoint class.
 */

#ifndef scolib_EApoint_h
#define scolib_EApoint_h

#include <acro_config.h>
#include <utilib/PackObject.h>
#include <scolib/DomainInfoBase.h>

namespace scolib {

using utilib::PackBuffer;
using utilib::UnPackBuffer;

///
template <class DomainT, class InfoT=DomainInfoBase>
class EApoint : public utilib::PackObject
{
public:

  ///
  EApoint() {}

  ///
  EApoint(const EApoint<DomainT,InfoT>& pt)
		{*this = pt;}

  ///
  virtual ~EApoint() {}

  ///
  virtual EApoint<DomainT,InfoT>& operator=(const EApoint<DomainT,InfoT>& pt)
		{
		point = pt.point;
		return *this;
		}

  ///
  virtual bool operator==(const EApoint<DomainT,InfoT>& pt) const
		{return (point == pt.point);}

  ///
  virtual void copy(const EApoint<DomainT,InfoT>& pt)
		{
		point << pt.point;
		info.copy(pt.info);
		}

  ///
  virtual void write_point(std::ostream& os) const
		{os << point;}

  ///
  virtual void ls_info(std::ostream&) const {}
 
  ///
  virtual void write(std::ostream& os) const
		{os << point << info;}

  ///
  virtual void read(std::istream& is)
		{is >> point >> info;}

  ///
  virtual void write(PackBuffer& pack) const
		{pack << point << info;}

  ///
  virtual void read(UnPackBuffer& unpack)
		{unpack >> point >> info;}

  ///
  DomainT point;

  ///
  InfoT info;

};

} // namespace scolib

#endif
