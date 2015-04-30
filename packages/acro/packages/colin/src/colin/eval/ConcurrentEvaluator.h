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
 * \file ConcurrentEvaluator.h
 *
 * Defines the \c ConcurrentEvaluator class.
 *
 **/

#ifndef colin_ConcurrentEvaluator_h
#define colin_ConcurrentEvaluator_h

#include <colin/EvaluationManager.h>
#include <colin/eval/SerialQueueManager.h>

namespace colin
{

class ConcurrentEvaluator : public EvaluationManager_Base
{
public:
   ConcurrentEvaluator(evalMngrID_t id);

   virtual ~ConcurrentEvaluator()
   {}

   static EvaluationManager_Base* create(size_t mngr_id)
   { return new ConcurrentEvaluator(mngr_id); }

   utilib::Property  max_concurrency;

protected:

   /// Perform an immediate (blocking) function evaluation
   virtual AppResponse perform_evaluation( solverID_t solver_id,
                                           AppRequest request );

   /// Queue up a function evaluation and return a (unique) ID
   virtual EvaluationID queue_evaluation( solverID_t solver_id,
                                          AppRequest request,
                                          evalPriority_t priority,
                                          queueID_t queue_id );

   ///
   virtual std::pair<EvaluationID, AppResponse> 
   next_response( solverID_t solver_id, queueID_t queue_id );

   ///
   virtual bool
   response_available( solverID_t solver_id, queueID_t queue_id );

   /// Wait for all function evaluations to complete
   virtual void synchronize( solverID_t solver_id, 
                             queueID_t queue_id,
                             bool keep_responses );

   /// Wait for all function evaluations to complete
   virtual void clear_evaluations(solverID_t solver_id, queueID_t queue_id);

   /// The number of processors available to solver (0 == all processors)
   virtual size_t num_evaluation_servers(solverID_t solver_id)
   {
      static_cast<void>(solver_id);
      return max_concurrency;
   }


private:  // methods
   void spin_lock();

   size_t collect_concurrent_processes();

   size_t 
   refill_concurrent_processes(solverID_t solver, 
                               queueID_t queue = QueueManager::ALL_SUBQUEUES);

   bool pending_evaluation_for_queue(solverID_t solver, queueID_t queue) const;

private:  // data
   struct PendingEvaluation {
      utilib::Any                  domain;
      AppRequest                   request;
      EvaluationID                 eval_id;
      Application_Base            *app;
      AppResponse::response_map_t  cache_hits;
      Cache::Key                   cache_key;
   };

   typedef std::pair<EvaluationID, AppResponse>  ResponseRecord;

   typedef std::map<solverID_t, std::list<ResponseRecord> > responseBuffer_t;

   typedef std::map<utilib::Any, PendingEvaluation>  pendingEvals_t;

   typedef std::map<Application_Base*, size_t>  runningApps_t;

   SerialQueueManager queue_mngr;

   bool queue_bypass_allowed;

   size_t current_concurrency;

   size_t polling_interval;

   responseBuffer_t responseBuffer;

   pendingEvals_t  pendingEvals;

   std::set<utilib::Any>  clearedEvals;

   runningApps_t  runningApps;
};


} // namespace colin

#endif // defined colin_ConcurrentEvaluator_h
