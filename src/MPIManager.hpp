/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        MPIManager
//- Description:  Class to manage  Dakota's parent MPI world
//- Owner:        Brian Adams
//- Version: $Id$

#ifndef DAKOTA_MPI_MANAGER_H
#define DAKOTA_MPI_MANAGER_H

#ifdef DAKOTA_HAVE_MPI
#include <mpi.h>
#endif // DAKOTA_HAVE_MPI


namespace Dakota {

#ifndef DAKOTA_HAVE_MPI
typedef int MPI_Comm;
typedef struct { int MPI_SOURCE; int MPI_TAG; int MPI_ERROR; } MPI_Status;
typedef void* MPI_Request;
#define MPI_COMM_WORLD   0
#define MPI_COMM_NULL    0
#define MPI_ANY_TAG      0
#define MPI_REQUEST_NULL 0
#endif // not DAKOTA_HAVE_MPI

class MPIManager {

public:

  MPIManager();
  MPIManager(int& argc, char**& argv);
  MPIManager(MPI_Comm dakota_mpi_comm);
  ~MPIManager();

  MPI_Comm dakota_mpi_comm() const { return dakotaMPIComm; }
  int world_rank() const { return worldRank; }
  int world_size() const { return worldSize; }
  int mpirun_flag() const { return mpirunFlag; }
  
  /// detect parallel launch of Dakota using mpirun/mpiexec/poe/etc.
  /// based on command line arguments and environment variables
  static bool detect_parallel_launch(int& argc, char**& argv);
 
private:

  MPI_Comm dakotaMPIComm; ///< MPI_Comm on which DAKOTA is running
  int worldRank;
  int worldSize;
  bool mpirunFlag;
  bool ownMPIFlag;

};  // class MPIManager

}  // namespace Dakota

#endif // DAKOTA_MPI_MANAGER_H
