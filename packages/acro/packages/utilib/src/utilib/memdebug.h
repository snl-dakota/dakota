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
 * \file memdebug.h
 *
 * Macros that can be used to debug memory allocation.
 * \note  This code is not very mature.
 *
 * \author William E. Hart
 */

#ifndef utilib_memdebug_h
#define utilib_memdebug_h

#if !defined(DOXYGEN)

#include <utilib_config.h>
#ifdef  UTILIB_YES_MEMDEBUG
#include <utilib/std_headers.h>

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif

#include <utilib/BasicArray.h>
#include <utilib/CharString.h>
#include <utilib/IntVector.h>


#define MEMDEBUG_START_NEW(this)					\
	{								\
	int i=0;							\
	for (; i<utilib::memdebug::num; i++)					\
          if (utilib::memdebug::name[i] == this) {				\
	     utilib::memdebug::last_total[i] = utilib::memdebug::nbytes;		\
	     break;							\
	     }								\
	if (i == utilib::memdebug::num) {					\
	   utilib::memdebug::num++;						\
	   if (utilib::memdebug::name.len() == utilib::memdebug::num) {			\
	      utilib::memdebug::name.resize(utilib::memdebug::num+10);			\
	      utilib::memdebug::num_new.resize(utilib::memdebug::num+10);		\
	      utilib::memdebug::num_del.resize(utilib::memdebug::num+10);		\
	      utilib::memdebug::memory_allocated.resize(utilib::memdebug::num+10);	\
	      utilib::memdebug::memory_deleted.resize(utilib::memdebug::num+10);	\
	      utilib::memdebug::last_total.resize(utilib::memdebug::num+10);		\
	      }								\
	   }								\
	utilib::memdebug::name[i] = this;					\
	utilib::memdebug::last_total[i] = utilib::memdebug::nbytes;			\
	}

#define MEMDEBUG_END_NEW(this)						\
	{								\
	for (int i=0; i<utilib::memdebug::num; i++)				\
          if (utilib::memdebug::name[i] == this) {				\
	     utilib::memdebug::memory_allocated[i] += 				\
			utilib::memdebug::nbytes - utilib::memdebug::last_total[i];	\
	     utilib::memdebug::num_new[i]++;					\
	     break;							\
	     }								\
	}


#define MEMDEBUG_START_RESIZE(this)					\
	{								\
	int i=0;							\
	for (; i<utilib::memdebug::num; i++)					\
          if (utilib::memdebug::name[i] == this)				\
	     utilib::memdebug::last_total[i] = utilib::memdebug::nbytes;		\
	}

#define MEMDEBUG_END_RESIZE(this)					\
	{								\
	for (int i=0; i<utilib::memdebug::num; i++)				\
          if (utilib::memdebug::name[i] == this) {				\
	     utilib::memdebug::memory_allocated[i] += 				\
			utilib::memdebug::nbytes - utilib::memdebug::last_total[i];	\
	     }								\
	}


#define MEMDEBUG_START_DEL(this)					\
	{								\
	}

#define MEMDEBUG_END_DEL(this)						\
	{								\
	for (int i=0; i<utilib::memdebug::num; i++)				\
          if (utilib::memdebug::name[i] == this) {				\
	     utilib::memdebug::num_del[i]++;					\
	     }								\
	}


#define MEMDEBUG_DUMP(os)		utilib::memdebug::print_summary(os);

namespace utilib {

class memdebug
{
public:

  static int nbytes;
  static int num;
  static int n_news;
  static int n_dels;

  static BasicArray<CharString> name;
  static IntVector 	       num_new;
  static IntVector 	       memory_allocated;
  static IntVector 	       num_del;
  static IntVector 	       memory_deleted;
  static IntVector 	       last_total;

#ifdef UTILIB_HAVE_NAMESPACES
  static void print_summary(std::ostream& os);
#else
  static void print_summary(ostream& os);
#endif
};

} // namespace utilib

#else

#define MEMDEBUG_START_NEW(this)
#define MEMDEBUG_END_NEW(this)	
#define MEMDEBUG_START_RESIZE(this)	
#define MEMDEBUG_END_RESIZE(this)
#define MEMDEBUG_START_DEL(this)
#define MEMDEBUG_END_DEL(this)
#define MEMDEBUG_DUMP(os)

#endif


#endif
#endif
