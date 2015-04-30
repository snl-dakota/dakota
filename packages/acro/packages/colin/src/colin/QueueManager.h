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
 * \file QueueManager.h
 *
 * Defines the colin::QueueManager class.  This provides the
 * multi-solver prioritized queue management system for serial
 * computations.
 */

#ifndef colin_QueueManager_h
#define colin_QueueManager_h

#include <acro_config.h>
#include <utilib/std_headers.h>
//#include <colin/AppRequest.h>


namespace colin
{

class EvaluationID;
class AppRequest;

class QueueManager
{
public:
   typedef size_t  evalMngrID_t;
   typedef size_t  solverID_t;
   typedef size_t  queueID_t;
   typedef double  evalPriority_t;
   typedef std::map<solverID_t, double>  solverAlloc_t;
   typedef std::map<queueID_t, double>   queueAlloc_t;

   static const solverID_t ALL_SOLVERS;
   static const queueID_t  ALL_SUBQUEUES;
   static const queueID_t  NO_SUBQUEUE;

public:

   /// Empty virtual destructor to silence compiler warnings
   virtual ~QueueManager() {}

   // --- Solver management

   /// Create a queue for a new solver and reallocate solver-level allocations
   virtual solverID_t get_new_solver_queue() = 0;

   /// Release an existing solver queue (clears all queued evaluations)
   virtual void release_solver_queue(solverID_t solver_id) = 0;

   /// Get the current normalized per-solver allocation
   virtual void get_solver_alloc(solverAlloc_t &alloc) const = 0;

   /// Get the current normalized allocation for a specific solver
   virtual double get_solver_alloc(solverID_t solver) const = 0;

   /// Set the allocation for all solvers
   virtual void set_solver_alloc(const solverAlloc_t &alloc) = 0;

   // --- Sub-queue management

   /// Get a new sub-queue id for the given solver
   virtual queueID_t get_new_subqueue(solverID_t solver_id) = 0;

   ///
   virtual void release_subqueue(solverID_t solver_id, queueID_t queue_id) = 0;

   ///
   virtual void get_subqueue_alloc( solverID_t solver_id, 
                                    queueAlloc_t &alloc ) const = 0;

   ///
   virtual void set_subqueue_alloc( solverID_t solver_id, 
                                    const queueAlloc_t &alloc ) = 0;

   // --- Evaluation queuing and management

   virtual void clear_evaluations( solverID_t solver_id, 
                                   queueID_t queue_id ) = 0;

   virtual EvaluationID queue_evaluation( AppRequest &request,
                                          evalMngrID_t evalMngr_id,
                                          solverID_t solver_id,
                                          queueID_t queue_id,
                                          evalPriority_t priority ) = 0;

   virtual bool queue_empty( solverID_t solver_id,
                             queueID_t queue_id ) const = 0;
};


} // namespace colin

#endif // colin_QueueManager_h
