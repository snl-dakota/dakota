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


//
// A simple test routine for MPI
//
#include <utilib_config.h>
#include <utilib/mpiUtil.h>
#include <utilib/CharString.h>

using namespace utilib;

int test_mpitest(int , char** )
{
#ifdef UTILIB_HAVE_MPI
   // Taken care of by the driver function now
   // uMPI::init(&argc,&argv);

	int rank = uMPI::rank;
	int size = uMPI::size;
	std::cout << "My rank is " << rank << " out of " << size << std::endl;
	if (rank == 0) {
	uMPI::send(&rank, 1, MPI_INT, rank+1, 0);
	}

	int buf;
	MPI_Status status;
	if (rank > 0) {
	   int dest = rank-1;
	   std::cout << "Processor " << rank << " waiting for " << dest << std::endl;
	   uMPI::recv((void*)&buf, 1, MPI_INT, dest, 0, &status);
	   }
	else {
	   int dest = size-1;
	   std::cout << "Processor " << rank << " waiting for " << dest << std::endl;
	   uMPI::recv((void*)&buf, 1, MPI_INT, dest, 0, &status);
	   }
	std::cout << "Processor " << rank << " received a message from processor " << buf << std::endl;
	if (rank == (size-1))
	   uMPI::send(&rank, 1, MPI_INT, 0, 0);
	else
	   uMPI::send(&rank, 1, MPI_INT, rank+1, 0);

   // Taken care of by the driver function now
   // uMPI::done();

#else
	std::cerr << "MPI is not configured right now!" << std::endl;
#endif
	return 0;
}
