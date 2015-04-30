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
 * \file enum_def.h
 *
 * A macro that defines stream operators for and enum type.
 */

#ifndef utilib_enum_def_h
#define utilib_enum_def_h

#include <utilib_config.h>
#include <utilib/PackBuf.h>

/// A macro to setup stream operators for packing and unpacking enumerated data.
#define ENUM_STREAMS(Etype)\
\
inline utilib::PackBuffer& operator<< (utilib::PackBuffer& buff, const Etype& data)\
{buff.pack( static_cast<int>(data) ); return buff;}\
\
inline utilib::UnPackBuffer& operator>> (utilib::UnPackBuffer& buff, Etype& data)\
{int tmp; buff.unpack(tmp); data = static_cast< Etype >(tmp); return buff;}\
\
inline std::ostream& operator<< (std::ostream& ostr, const Etype data)\
{ostr << static_cast<int>(data); return ostr;}\
\
inline std::istream& operator>> (std::istream& istr, Etype& data)\
{int tmp; istr >> tmp; data = static_cast< Etype >(tmp); return istr;}

#endif
