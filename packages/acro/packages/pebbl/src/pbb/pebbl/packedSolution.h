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
 * \file packedSolution.h
 * \author Jonathan Eckstein
 *
 * A helper class for the PEBBL parallel layer.  Stores a solution in
 * packed form.  Used only when enumerating with flow control enabled.
 */

#ifndef pebbl_packedSolution_h
#define pebbl_packedSolution_h


#include <acro_config.h>
#include <utilib/mpiUtil.h>
#include <pebbl/branching.h>
#include <pebbl/parPebblBase.h>


namespace pebbl {

  class packedSolution : public parallelPebblBase
    {
    public:

      int    owner;   // Processor where this solution wants to go
      double value;   // Objective value (negated for max problems)

      PackBuffer buf; // Packed message form of the solution

      // Constructor that starts from a regular solution

      packedSolution(int owner_, double value_,solution* sol) :
	owner(owner_),
	value(value_)
	{
	  buf << (int) forwardSolSignal << owner << value;
	  sol->pack(buf);
	};

      // Constructor that uses an UnPackBuffer

      packedSolution(int owner_, double value_,UnPackBuffer& inBuf) :
	owner(owner_),
	value(value_),
	buf(inBuf)
	{ };

      // Comparison operator (which assumes minimization; for max
      // problems, the 'value' members are negated objective values)

      int compare(const packedSolution& other) const
	{
	  return sgn(value - other.value);
	};

      // Send the solution to a designated processor (with no request
      // object left over)

      void isend(int dest,int tag)
	{
	  uMPI::isend((void *) buf.buf(),buf.curr(),MPI_PACKED,dest,tag);
	};

    };

} // namespace pebbl

#endif
