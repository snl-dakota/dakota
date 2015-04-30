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
 * \file MasterSlaveOptExec.h
 *
 * Defines a function that launches a COLIN optimizer with master-slave
 * parallelism (using MPI).
 */

#ifndef colin_MasterSlaveOptExec_h
#define colin_MasterSlaveOptExec_h

#include <acro_config.h>
#include <colin/Solver.h>
#include <colin/MSApplication.h>

#if defined(USING_PROBE)
extern void probe();
#endif

namespace colin
{

/// Execute a simple master-slave optimization process.
template <class SolverT, class DomainT, class ResponseT>
void MasterSlaveOptExec(SolverT & opt, Problem<DomainT, ResponseT> & prob,
                        int batch_size = 1,
                        bool eval_on_master = false, bool debug_flag = false)
{
#ifdef ACRO_HAVE_MPI
//
// Check if you need to run MPI for the user
//
   int local_mpi;
   MPI_Initialized(&local_mpi);
   if (!local_mpi)
      EXCEPTION_MNGR(std::runtime_error, "MasterSlaveOptExec - MPI has not been initialized!");


      int rank, size;
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);   // Compute processor id
   MPI_Comm_size(MPI_COMM_WORLD, &size);   // Compute # of processors

   if ((size == 1) && (eval_on_master == FALSE))
      EXCEPTION_MNGR(std::runtime_error, "Attempting to run MasterSlaveOptExec with only 1 processor!");

#if defined(USING_PROBE)
      probe();
#endif
   MSApplication<DomainT, ResponseT> *app = new MSApplication<DomainT, ResponseT>(prob, batch_size, eval_on_master);
   app->copy(prob.get_application());
   app->problem &= prob;
   app->debug = debug_flag;

//
// Run the master process
//
   if (rank == 0)
   {
      if (debug_flag)
      {
         ucout << "Performing master-slave optimization with "
         << size << " processors" << std::endl;
         ucout.flush();
      }

      ucout << "Num servers: " << prob.num_evaluation_servers() << std::endl;
      ucout << "Num real params: " << prob.num_real_params() << std::endl;
      opt.set_problem(prob);
      prob.set_application(app, true);
      ucout << "Num servers: " << prob.num_evaluation_servers() << std::endl;
      ucout << "Num real params: " << prob.num_real_params() << std::endl;
      opt.set_problem(prob);
      ucout << "Num servers: " << opt.get_problem().num_evaluation_servers() << std::endl;
      ucout << "Num real params: " << opt.get_problem().num_real_params() << std::endl;
      opt.reset();
      opt.minimize();
      //opt.set_problem(tmp_prob);

      if (debug_flag)
         (ucout << "Process " << rank << " is terminating now." << std::endl).flush();
      app->terminate();
   }

//
// Run the slave process
//
   else
   {
      app->exec_server(prob);

      if (debug_flag)
         (ucout << "Process " << rank << " is terminating now." << std::endl).flush();
      delete app;
   }

#endif
}

}

#endif
