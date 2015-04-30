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
 * \file MessageID.h
 *
 * Defines the pebbl::MessageID class.
 */

#ifndef pebbl_MessageID_h
#define pebbl_MessageID_h

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

namespace pebbl {


///
/// This class provides a mechanism for allocating message ids in a manner
/// which allows library development of parallel routines.  Upon 
/// construction, the MessageID object reserves the next available message 
/// tag (a nonnegative integer).  This class can be coerced to an integer 
/// to get the tag value.  This class does not support recycling of message 
/// tags when MessageID classes are deleted, but a global reset method is
/// provided to enable reallocating of MessageIDs.
///
class MessageID
{
public:

	///
  MessageID(MessageID& mID)
	{id = mID.id;}
	///
  MessageID(int id_)
	{id = id_;}
	///
  MessageID()
	{id = next_id++;}

	///
  operator int() const {return id;}
	///
  MessageID& operator=(const MessageID& mID)
	{id = mID.id; return *this;}

	///
  static int check_id();
	///
  static void reset_ids();

private:

	///
  static int next_id;
	///
  int id;
};

extern MessageID NullMsg;
extern MessageID AnyMsg;

} // namespace pebbl

#endif

#endif
