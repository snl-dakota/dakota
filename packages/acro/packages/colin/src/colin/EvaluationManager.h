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
 * \file EvaluationManager.h
 *
 * Defines the colin::EvaluationManager (solver-specific interface to
 * the evaluation management system) and colin::EvaluationManager_Base
 * (the multi-solver evaluation management system) classes.
 */

#ifndef colin_EvaluationManager_h
#define colin_EvaluationManager_h

#include <colin/EvaluationManagerFactory.h>
#include <colin/QueueManager.h>
#include <colin/EvaluationID.h>
#include <colin/ResponseGenerator.h>
#include <colin/Cache.h>

#include <utilib/PropertyDict.h>

//#include <colin/TinyXML.h>
class TiXmlElement;

namespace colin {

/// The "sense" for the optimization (minimixe / maximize)
enum optimizationSense
   {
      minimization = 1,
      unknown = 0,
      maximization = -1
   };

} // namespace colin
namespace utilib {

DEFINE_DEFAULT_ANY_PRINTER(colin::optimizationSense);
DEFINE_DEFAULT_ANY_COMPARATOR(colin::optimizationSense);

} // namespace utilib

namespace colin {

class EvaluationManager;
class EvaluationManager_Base;


/// Define a handle to a (derived) Application class
typedef Handle<EvaluationManager_Base> EvaluationManager_Handle;


/** The base class for all Evaluation Manager implementations.
 *  Evaluation managers rely on a common fundamental structure ("set of
 *  sets of priority queues") for organizing function evaluation
 *  requests.  Each solver is allowed to define a set of priority
 *  queues.  Computational effort is allocated amont the individual
 *  solvers using set_solver_alloc().  Within each solver's allocation,
 *  computational effort for the solver's individual queues is allocated
 *  through set_queue_alloc().
 *
 *  For consistency, all Evaluation Managers (including serial
 *  evaluators) should strive to mimic the execution sequencing of an
 *  asynchronous parallel evaluation manager.
 */
class EvaluationManager_Base : public ResponseGenerator, 
                               public Handle_Client<EvaluationManager_Base>
{
   friend class EvaluationManager;

public:
   typedef QueueManager::evalMngrID_t    evalMngrID_t;
   typedef QueueManager::solverID_t      solverID_t;
   typedef QueueManager::queueID_t       queueID_t;
   typedef QueueManager::evalPriority_t  evalPriority_t;
   typedef QueueManager::solverAlloc_t   solverAlloc_t;
   typedef QueueManager::queueAlloc_t    queueAlloc_t;


public:

   ///
   EvaluationManager_Base(evalMngrID_t id, QueueManager &qmgr)
      : mngr_id(id),
        queue_mngr(qmgr),
        eval_cache()
   {}

   ///
   virtual ~EvaluationManager_Base();

   ///
   virtual void initialize(TiXmlElement* params)
   { static_cast<void>(params); }

   ///
   void get_solver_alloc(std::map<solverID_t, double> &alloc) const
   { queue_mngr.get_solver_alloc(alloc); }

   ///
   void set_solver_alloc(const std::map<solverID_t, double> &alloc)
   { queue_mngr.set_solver_alloc(alloc); }

   ///
   void set_evaluation_cache(CacheHandle new_cache);

   ///
   CacheHandle evaluation_cache() const
   { return eval_cache; }

   /// True if the evaluation manager has the named property
   bool has_property( std::string name )
   { return properties.exists(name); }

   /// get a reference to the named property
   utilib::Property& property( std::string name );

   /// get a reference to the named property
   utilib::Property& operator[]( std::string name )
   { return property(name); }

   enum PropertyIteratorLocation {
      BEGIN = 0,
      END = 1
   };

   /// Return an iterator to the underlying Property dictionary
   utilib::PropertyDict::const_iterator 
   propertyIterator(PropertyIteratorLocation location = BEGIN ) const
   {
      return location ? properties.end() : properties.begin();
   }

protected: // but exposed through EvaluationManager

   /// Perform an immediate (blocking) function evaluation
   virtual AppResponse perform_evaluation(solverID_t solver_id,
                                          AppRequest request) = 0;

   /// Queue up a function evaluation and return a (unique) ID
   virtual EvaluationID queue_evaluation(solverID_t solver_id,
                                         AppRequest request,
                                         evalPriority_t priority,
                                         queueID_t queue_id) = 0;

   /// Return the next response that has been computed
   virtual std::pair<EvaluationID, AppResponse> 
   next_response(solverID_t solver_id, queueID_t queue_id) = 0;

   /// Queue up a function evaluation and return a (unique) ID
   virtual bool 
   response_available(solverID_t solver_id, queueID_t queue_id) = 0;


   /// Wait for all function evaluations to complete
   /** WEH - how does this related to the execute_evaluations() method
    *        supported by batch evaluator?
    */
   virtual void synchronize(solverID_t solver_id, queueID_t queue_id,
                            bool keep_responses) = 0;

   /// Clear all outstanding evaluations
   virtual void clear_evaluations(solverID_t solver_id,
                                  queueID_t queue_id) = 0;

   /// The number of processors available to solver (0 == all processors)
   virtual size_t num_evaluation_servers(solverID_t solver_id) = 0;


   /// Get a new queue id for the given solver
   queueID_t get_new_queue_id(solverID_t solver_id)
   { return queue_mngr.get_new_subqueue(solver_id); }

   ///
   void release_queue(solverID_t solver_id, queueID_t queue_id)
   { queue_mngr.release_subqueue(solver_id, queue_id); }

   ///
   void get_queue_alloc(solverID_t solver_id, queueAlloc_t &alloc) const
   { queue_mngr.get_subqueue_alloc(solver_id, alloc); }

   ///
   void set_queue_alloc(solverID_t solver_id,const queueAlloc_t &alloc)
   { queue_mngr.set_subqueue_alloc(solver_id, alloc); }


protected: // and NOT exposed through EvaluationManager
   /// Get a new solver id - this sets up a queue for a new solver 
   solverID_t reference_solver(solverID_t existing = 0);

   /// Release a solver ID (purge queue, remove from allocation lists)
   void release_solver(solverID_t solver_id);

   ///
   utilib::Any spawn_core_request(CoreRequestInfo &cqi) const;

   ///
   CoreResponseInfo collect_core_response( Application_Base *app ) const;

   ///
   CoreResponseInfo perform_core_evaluation( CoreRequestInfo &cqi ) const;

   ///
   bool core_response_available( Application_Base *app ) const;

   ///
   Cache::Key    
   request_check_cache( bool force_recalc, CoreRequestInfo &cqi, 
                        AppResponse::response_map_t &cache_hits ) const;
   

protected: // data

   /// a rank-unique id for this manager (assigned by EvalManagerFactory)
   size_t  mngr_id;

   /// a reference to the QueueManager I should be using
   QueueManager& queue_mngr;

   /// a pointer to the evaluation cache I should be using
   CacheHandle eval_cache;

   /// The list of active solvers
   std::map<solverID_t, size_t>  solver_refCount;

   /// All properties registered by derived classes
   utilib::PropertyDict properties;
};



/** A wrapper class so that Solvers can be ignorant about other solvers
 *  that happen to be using the same evaluation manager (i.e. the case
 *  where we have multiple optimization algorithms operating in
 *  parallel).  This class wraps an EvaluationManager_base (or
 *  derivative) class and masks the solver_id parameter from the calling
 *  functions.
 */
class EvaluationManager
{
public:
   typedef QueueManager::evalMngrID_t    evalMngrID_t;
   typedef QueueManager::solverID_t      solverID_t;
   typedef QueueManager::queueID_t       queueID_t;
   typedef QueueManager::evalPriority_t  evalPriority_t;
   typedef QueueManager::solverAlloc_t   solverAlloc_t;
   typedef QueueManager::queueAlloc_t    queueAlloc_t;

public:
   ///
   EvaluationManager()
      : mngr()
   {}

   ///
   EvaluationManager(const EvaluationManager_Handle rhs_mngr)
      : mngr(rhs_mngr)
   { solver_id = mngr->reference_solver(); }

   ///
   EvaluationManager(const EvaluationManager& rhs)
      : mngr(),
        solver_id(0)
   { *this = rhs; }

   ///
   EvaluationManager& operator=(const EvaluationManager_Handle &rhs)
   {
      if ( mngr.object() == rhs.object() )
         return *this;

      if ( ! mngr.empty() )
         mngr->release_solver(solver_id); 
      mngr = rhs;
      solver_id = mngr->reference_solver(/*rhs.solver_id*/);
      return *this;
   }

   ///
   EvaluationManager& operator=(const EvaluationManager &rhs)
   {
      return *this = rhs.handle();
   }

   ///
   ~EvaluationManager()
   { 
      if ( ! mngr.empty() ) 
         mngr->release_solver(solver_id); 
   }

   const EvaluationManager_Handle handle() const
   { return mngr; }

   /// True if the EvaluationManager refers to a valid Eval Manager
   operator bool() const
   { return ! mngr.empty(); }

   solverID_t solverID() const
   { return solver_id; }

   ///
   CacheHandle evaluation_cache() const
   { return mngr.empty() ? CacheHandle() : mngr->eval_cache; }

   /// Perform an immediate (blocking) function evaluation
   AppResponse perform_evaluation(AppRequest request) const
   {
      if ( mngr.empty() )
      {
         EXCEPTION_MNGR(std::runtime_error, "EvaluationManager::"
                        "perform_evaluation - no manager object allocated.");
      }
      return mngr->perform_evaluation(solver_id, request);
   }

   /// Queue up a function evaluation and return a (unique) ID
   EvaluationID queue_evaluation( AppRequest request,
                                  evalPriority_t priority = 0,
                                  queueID_t queue_id = 0 ) const
   {
      if ( mngr.empty() )
      {
         EXCEPTION_MNGR(std::runtime_error, "EvaluationManager::"
                        "queue_evaluation - no manager object allocated.");
      }
      return mngr->queue_evaluation(solver_id, request,
                                    priority, queue_id);
   }

   /// Return the next response that has been computed
   std::pair<EvaluationID, AppResponse> 
   next_response(queueID_t queue_id = 0) const
   {
      if ( mngr.empty() )
      {
         EXCEPTION_MNGR(std::runtime_error, "EvaluationManager::"
                        "next_response - no manager object allocated.");
      }
      return mngr->next_response(solver_id, queue_id);
   }

   /// Another wrapper for computing a next_response, which requires
   /// explicit id and response parameters.
   void next_response( EvaluationID& id, AppResponse& response, 
                       queueID_t queue_id = 0 ) const
   {
      std::pair<EvaluationID, AppResponse> ans = next_response(queue_id);
      id = ans.first;
      response = ans.second;
   }

   /// Return true if a computed response is available through next_response
   bool response_available(queueID_t queue_id = 0) const
   {
      if ( mngr.empty() )
      {
         EXCEPTION_MNGR(std::runtime_error, "EvaluationManager::"
                        "response_available - no manager object allocated.");
      }
      return mngr->response_available(solver_id, queue_id);
   }

   /// Wait for all function evaluations to complete
   void synchronize(queueID_t queue_id = 0, bool keep_responses = false) const
   { 
      if ( ! mngr.empty() ) 
         mngr->synchronize(solver_id, queue_id, keep_responses); 
   }

   /// Clear all outstanding evaluations
   void clear_evaluations(queueID_t queue_id = 0) const
   {
      if ( ! mngr.empty() ) 
         mngr->clear_evaluations(solver_id, queue_id); 
   }

   /// Flush queue and return first better point (SO only)
   EvaluationID find_first_improving(AppResponse& response,
                                     utilib::AnyRef target,
                                     optimizationSense sense = minimization,
                                     queueID_t queue_id = 0) const;

   /// Flush queue and return best point (SO only)
   EvaluationID find_best_improving(AppResponse& response,
                                    utilib::AnyRef target,
                                    optimizationSense sense = minimization,
                                    queueID_t queue_id = 0) const;

   ///
   size_t num_evaluation_servers() const
   {
      if ( mngr.empty() )
      {
         EXCEPTION_MNGR(std::runtime_error, "EvaluationManager::"
                        "num_evaluation_servers - no manager object "
                        "allocated.");
      }
      return mngr->num_evaluation_servers(solver_id);
   }

   ///
   size_t total_num_evaluation_servers() const
   {
      if ( mngr.empty() )
      {
         EXCEPTION_MNGR(std::runtime_error, "EvaluationManager::"
                        "total_num_evaluation_servers - no manager object "
                        "allocated.");
      }
      return mngr->num_evaluation_servers(0);
   }

   /// Get a new queue id for the given solver
   queueID_t get_new_queue_id() const
   {
      if ( mngr.empty() )
      {
         EXCEPTION_MNGR(std::runtime_error, "EvaluationManager::"
                        "get_new_queue_id - no manager object allocated.");
      }
      return mngr->get_new_queue_id(solver_id);
   }

   void release_queue(queueID_t queue_id = 0) const
   { return mngr->release_queue(solver_id, queue_id); }

   void get_queue_alloc(std::map<queueID_t, double> &alloc) const
   { mngr->get_queue_alloc(solver_id, alloc); }

   void set_queue_alloc(const std::map<queueID_t, double> &alloc) const
   { mngr->set_queue_alloc(solver_id, alloc); }

private:

   ///
   EvaluationManager_Handle  mngr;

   ///
   solverID_t  solver_id;
};



} // namespace colin



#endif // defined colin_EvaluationManager_h
