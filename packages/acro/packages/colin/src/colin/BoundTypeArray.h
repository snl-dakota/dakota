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
 * \file BoundTypeArray.h
 *
 * Defines enumeration types for bounds.
 */

#ifndef __colin_BoundTypeArray_h
#define __colin_BoundTypeArray_h

#include <acro_config.h>
#include <utilib/EnumBitArray.h>
#include <utilib/enum_def.h>

namespace colin
{

#if !defined(DOXYGEN)
enum bound_type_enum 	{
   no_bound = 0,
   hard_bound = 1,
   soft_bound = 2,
   periodic_bound = 3
};

typedef utilib::EnumBitArray<1, bound_type_enum> BoundTypeArray;

inline void convert_to_char(bound_type_enum type, char& c)
{
   if (type == no_bound) {
 	c='N';
	return;
	}
   if (type == hard_bound) {
 	c='H';
	return;
	}
   if (type == soft_bound) {
 	c='S';
	return;
	}
   if (type == periodic_bound) {
 	c='P';
	return;
	}
}

inline void convert(const char* str, bound_type_enum& type)
{
   if ((strcmp(str, "no") == 0) || (strcmp(str, "none")==0))
      type = no_bound;
   else if (strcmp(str, "hard") == 0)
      type = hard_bound;
   else if (strcmp(str, "soft") == 0)
      type = soft_bound;
   else if (strcmp(str, "periodic") == 0)
      type = periodic_bound;
   else {
      EXCEPTION_MNGR(std::runtime_error, "colin::convert - unknown bound type: \"" << str << "\"");
   }
}

inline void convert(std::string& str, bound_type_enum& type)
{ convert(str.data(), type); }

}

ENUM_STREAMS(colin::bound_type_enum)

inline std::ostream& operator<<(std::ostream& os, const colin::BoundTypeArray& array)
{
   os << array.size() << " : ";
   for (size_t i = 0; i < array.size(); i++)
   {
      colin::bound_type_enum tmp = array[i];
      switch (tmp)
      {
      case colin::no_bound:
         os << "N"; break;
      case colin::hard_bound:
         os << "H"; break;
      case colin::soft_bound:
         os << "S"; break;
      case colin::periodic_bound:
         os << "P"; break;
      };
   }
   return os;
}

#endif

#endif
