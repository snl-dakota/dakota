/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        MPIManager
//- Description:  Class to manage Dakota's parent MPI world
//- Owner:        Brian Adams
//- Version: $Id$

#ifndef DAKOTA_MPI_MANAGER_H
#define DAKOTA_MPI_MANAGER_H

#ifdef DAKOTA_HAVE_MPI
#include <mpi.h>
#endif // DAKOTA_HAVE_MPI


namespace Dakota {

#ifndef DAKOTA_HAVE_MPI
// These are constants in the Dakota namespace instead of preprocessor
// defines to avoid collisions with symbols defined by library clients.
typedef int MPI_Comm;
typedef struct { int MPI_SOURCE; int MPI_TAG; int MPI_ERROR; } MPI_Status;
typedef void* MPI_Request;
static const int MPI_COMM_WORLD   = 1;
static const int MPI_COMM_NULL    = 0;
// From older MPICH (could use 1140850689 = 0x44000001 from newer MPICH):
static const int MPI_COMM_SELF    = 92;
static const int MPI_ANY_TAG      = -1;
static void*     MPI_REQUEST_NULL = NULL;
#endif // not DAKOTA_HAVE_MPI


/// Class MPIManager to manage Dakota's MPI world, which may be a
/// subset of MPI_COMM_WORLD
class MPIManager {

public:

  /// Default constructor; Dakota will not call MPI_Init
  MPIManager();
  /// Command-line constructor; parses MPI arguments during call to MPI_Init 
  MPIManager(int& argc, char**& argv);
  /// Construct on specified MPI_Comm
  MPIManager(MPI_Comm dakota_mpi_comm);
  /// destructor: calls finalize if Dakota owns MPI
  ~MPIManager();

  /// get the MPI_Comm on which Dakota is running
  MPI_Comm dakota_mpi_comm() const;
  /// get the rank of this process in Dakota's MPI_Comm
  int world_rank() const;           
  /// get the size of the MPI_Comm on which Dakota is running
  int world_size() const;
  /// true when Dakota is running in MPI mode
  bool mpirun_flag() const;
  
  /// detect parallel launch of Dakota using mpirun/mpiexec/poe/etc.
  /// based on command line arguments and environment variables
  static bool detect_parallel_launch(int& argc, char**& argv);
 
private:

  MPI_Comm dakotaMPIComm; ///< MPI_Comm on which DAKOTA is running
  int dakotaWorldRank;    ///< rank in MPI_Comm in which DAKOTA is running
  int dakotaWorldSize;    ///< size of MPI_Comm in which DAKOTA is running
  bool mpirunFlag;        ///< flag for a parallel mpirun/yod launch
  bool ownMPIFlag;        ///< flag for ownership of MPI_Init/MPI_Finalize

};  // class MPIManager


inline MPI_Comm MPIManager::dakota_mpi_comm() const 
{ return dakotaMPIComm; }

inline int MPIManager::world_rank() const
{ return dakotaWorldRank; }

inline int MPIManager::world_size() const
{ return dakotaWorldSize; }

inline bool MPIManager::mpirun_flag() const 
{ return mpirunFlag; }

}  // namespace Dakota

#endif // DAKOTA_MPI_MANAGER_H
