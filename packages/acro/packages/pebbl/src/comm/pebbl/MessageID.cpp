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
//
// MessageID.cpp
//

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <utilib/std_headers.h>
#include <mpi.h>
#include <utilib/mpiUtil.h>
#include <pebbl/MessageID.h>

namespace pebbl {

using utilib::uMPI;

int MessageID::next_id=0;

MessageID NullMsg;
MessageID AnyMsg(MPI_ANY_TAG);


int MessageID::check_id()
{
int tmp;
int flag;
MPI_Attr_get(uMPI::comm, MPI_TAG_UB, (void*)&tmp, &flag);
assert(flag == 1);

return (next_id < tmp);
}


void MessageID::reset_ids()
{
//
// Reset the message ID number to the initial value.  Note that we assume that 
// this is 1, since the NullMsg uses the zero id.
//
next_id = 1;
}

} // namespace pebbl

#endif
