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
 * \file LocalQueueManager.h
 *
 * Defines the colin::LocalQueueManager class.
 */

#ifndef colin_LocalQueueManager_h
#define colin_LocalQueueManager_h

#include <acro_config.h>
//#include <utilib/std_headers.h>
#include <colin/QueueManager.h>
#include <colin/EvaluationID.h>
#include <colin/AppRequest.h>


namespace colin
{

class LocalQueueManager : public QueueManager
{
public:
   LocalQueueManager();
   ~LocalQueueManager();

   // --- Solver management

   /// Create a queue for a new solver and reallocate solver-level allocations
   virtual solverID_t get_new_solver_queue();

   /// Release an existing solver queue (clears all queued evaluations)
   virtual void release_solver_queue(solverID_t solver_id);

   /// Get the current normalized per-solver allocation
   virtual void get_solver_alloc(solverAlloc_t &alloc) const;

   /// Get the current normalized allocation for a specific solver
   virtual double get_solver_alloc(solverID_t solver) const;

   /// Set the allocation for all solvers
   virtual void set_solver_alloc(const solverAlloc_t &alloc);

   // --- Sub-queue management

   /// Get a new sub-queue id for the given solver
   virtual queueID_t get_new_subqueue(solverID_t solver_id);

   ///
   virtual void release_subqueue(solverID_t solver_id, queueID_t queue_id);

   ///
   virtual void get_subqueue_alloc( solverID_t solver_id,
                                    queueAlloc_t &alloc ) const;

   ///
   virtual void set_subqueue_alloc( solverID_t solver_id,
                                    const queueAlloc_t &alloc );

   // --- Evaluation queuing and management

   virtual bool queue_empty( solverID_t solver_id,
                             queueID_t queue_id ) const;

protected:
   struct SolverInfo;
   struct SubQueueInfo;
   struct RequestRecord;
   typedef std::map<solverID_t, SolverInfo>    queueMap_t;
   typedef std::map<queueID_t,  SubQueueInfo>  subqueueMap_t;
   typedef std::map<evalPriority_t, std::list<RequestRecord> >  priorityMap_t;

   struct SubQueueInfo {
      SubQueueInfo()
         : allocation(0)
      {}
      
      double         allocation;
      priorityMap_t  pqueues;
   };
   
   struct SolverInfo {
      SolverInfo()
         : allocation(0)
      {}
      
      double         allocation;
      subqueueMap_t  subqueues;
   };

   struct RequestRecord {
      RequestRecord(EvaluationID id, AppRequest& req)
         : evalID(id), request(req)
      {}

      EvaluationID evalID;
      AppRequest   request;
   };

protected:
   virtual void new_solver_alloc()
   {}

   virtual void new_queue_alloc(queueMap_t::iterator s_it)
   { static_cast<void>(s_it); }

   bool queue_empty_helper( queueMap_t::const_iterator s_it,
                            queueID_t queue_id ) const;


protected:
   queueMap_t  *m_queues;

private:
   solverID_t   m_lastSolverID;
   queueID_t    m_lastQueueID;
};


} // namespace colin

#endif // colin_LocalQueueManager_h
