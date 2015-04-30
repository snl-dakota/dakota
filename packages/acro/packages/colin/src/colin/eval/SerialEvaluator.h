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
 * \file SerialEvaluator.h
 *
 * Defines the \c SerialEvaluator class.
 *
 **/

#ifndef colin_SerialEvaluator_h
#define colin_SerialEvaluator_h

#include <colin/EvaluationManager.h>
#include <colin/eval/SerialQueueManager.h>

namespace colin
{

class SerialEvaluator : public EvaluationManager_Base
{
public:
   SerialEvaluator(evalMngrID_t id);

   virtual ~SerialEvaluator()
   {}

   static EvaluationManager_Base* create(size_t mngr_id)
   { return new SerialEvaluator(mngr_id); }

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
   virtual void clear_evaluations(solverID_t solver_id, queueID_t queue_id)
   { queue_mngr.clear_evaluations(solver_id, queue_id); }

   /// The number of processors available to solver (0 == all processors)
   virtual size_t num_evaluation_servers(solverID_t solver_id)
   {
      static_cast<void>(solver_id);
      return 1;
   }


private:  // data
   typedef std::pair<EvaluationID, AppResponse>  ResponseRecord;

   typedef std::map<solverID_t, std::list<ResponseRecord> > responseBuffer_t;


   SerialQueueManager queue_mngr;

   bool queue_bypass_allowed;

   responseBuffer_t responseBuffer;
};


} // namespace colin

#endif // defined colin_SerialEvaluator_h
