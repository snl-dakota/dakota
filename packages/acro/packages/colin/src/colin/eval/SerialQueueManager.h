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
 * \file SerialQueueManager.h
 *
 * Defines the colin::SerialQueueManager class.  This provides the
 * multi-solver prioritized queue management system for serial
 * computations.
 */

#ifndef colin_SerialQueueManager_h
#define colin_SerialQueueManager_h

#include <colin/eval/LocalQueueManager.h>

namespace colin
{

class SerialQueueManager : public LocalQueueManager
{
public:
   SerialQueueManager();

   virtual void clear_evaluations( solverID_t solver_id, 
                                   queueID_t queue_id );

   virtual EvaluationID queue_evaluation( AppRequest &request,
                                          evalMngrID_t evalMngr_id,
                                          solverID_t solver_id,
                                          queueID_t queue_id,
                                          evalPriority_t priority );

   EvaluationID get_next_request( AppRequest& request, 
                                  solverID_t solver_id, 
                                  queueID_t queue_id );

protected:
   virtual void new_queue_alloc(queueMap_t::iterator s_it);

private:
   typedef std::list<queueID_t> execSequence_t;
   typedef std::map<solverID_t, execSequence_t> execSequenceMap_t;

   /// Any allocation less than this tolerance is considered to be 0.
   static const double ZeroAlloc;

private:
   void generate_exec_sequence( execSequence_t &sequence, 
                                queueMap_t::iterator s_it );

   execSequenceMap_t execSequenceMap;
};

} // namespace colin
#endif // colin_SerialQueueManager_h
