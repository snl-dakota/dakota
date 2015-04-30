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
 * \file MultiStatePS.h
 *
 * Defines the scolib::MultiStatePS class.
 */

#ifndef scolib_MultiStatePS_h
#define scolib_MultiStatePS_h

#include <acro_config.h>
//#include <utilib/default_rng.h>
//#include <utilib/Normal.h>
#include <colin/solver/ColinSolver.h>
//#include <colin/AsyncEvaluator.h>
//#include <colin/ResponseSet.h>
#include <utilib/SmartPtr.h>

namespace scolib {

template<typename T>
class SmartPointer
{
private:
   class PtrContainer 
   {
   public:
      PtrContainer(T* ref)
         : data(ref),
           refCount(1)
      {}

      T*  data;
      int  refCount;
   };

public:
   SmartPointer()
      : m_ptr(NULL)
   { }
   SmartPointer(T* ref)
      : m_ptr(new PtrContainer(ref))
   { }
   SmartPointer(const SmartPointer<T> & rhs)
      : m_ptr(NULL)
   {
      operator=(rhs);
   }

   ~SmartPointer()
   { 
      deref();
   }

   SmartPointer<T>& operator=(T* ref)
   {
      deref();
      m_ptr = new PtrContainer(ref);
      return *this;
   }
   SmartPointer<T>& operator=(const SmartPointer<T> & rhs)
   {
      deref();
      m_ptr = rhs.m_ptr;
      if ( m_ptr != NULL )
      {
         ++(m_ptr->refCount);
      }
      return *this;
   }

   T* operator->() 
   {
      if ( m_ptr == NULL )
      {
         EXCEPTION_MNGR(std::runtime_error, "SmartPointer: dereferencing "
                        "empty SmartPointer.");
      }
      return m_ptr->data;
   }
   const T* operator->() const
   {
      if ( m_ptr == NULL )
      {
         EXCEPTION_MNGR(std::runtime_error, "SmartPointer: dereferencing "
                        "empty SmartPointer.");
      }
      return m_ptr->data;
   }


private:
   void deref()
   {
      if ( m_ptr != NULL )
      {
         if ( --(m_ptr->refCount) == 0 )
         {
            delete m_ptr->data;
            delete m_ptr;
         }
         m_ptr = NULL;
      }
   }

private:
   PtrContainer *m_ptr;
};


/** An implemention of a "stateless" pattern search algorithm that
 *  leverages the colin::AsyncEvaluator class.
 *
 *  NOTE: Currently, the MultiStatePS has NO termination criteria.  It
 *  is STRONGLY RECOMMENDED that you set max_nevals.
 */
class MultiStatePS 
   : public colin::ColinSolver<std::vector<double>, colin::UNLP0_problem>
{
public: // typedefs & sub-classes
   typedef std::vector<double>  domain_t;
   typedef colin::AppRequest    request_t;
   typedef colin::AppResponse   response_t;
   typedef int                  queueSet_t;
   typedef colin::QueueManager::queueID_t       pseudoQueue_t;
   typedef colin::QueueManager::evalPriority_t  evalPriority_t;
   typedef colin::ColinSolver<domain_t, colin::UNLP0_problem>
      ConcreteOptSolver_t;

   class PatternState;
   typedef SmartPointer<PatternState>  StateContainer;

   /// This structure holds all of the pattern search's configuration
   /// data.  Each MultiStatePS holds a private copy of this structure,
   /// and gives a const pointer to the struct to each PatternState
   struct MultiStatePatternConfig 
   {
      MultiStatePatternConfig()
         : contraction_factor(0.5),
           delta_init(1),
           delta_thresh(1e-5),
           expansion_factor(2),
           sufficient_decrease_coef(0.01),
           max_success(5),
           sigma(1,1)
      {}

      double contraction_factor;
      double delta_init;
      double delta_thresh;
      double expansion_factor;
      double sufficient_decrease_coef;
      int    max_success;
      std::vector<double> sigma;
   };

public: // methods
   ///
   MultiStatePS();

   ///
   virtual void optimize();

   ///
   virtual void write(std::ostream& os) const;


   /// Return a response set for the given state (point)
   response_t checkCache(domain_t &point);

   /// Queue an evaluation associated with a given state
   colin::EvaluationID queue_state_eval( StateContainer state, 
                                         request_t request,
                                         evalPriority_t priority,
                                         pseudoQueue_t queue );

   /// Generate the exploration points around a given point
   void expand_pattern_cartesian( response_t  newCenter, double step,
                                  std::list<request_t> & new_pts );


   /// get a new pattern ID
   long get_next_pattern_id()
   {
      static long lastStateID = 0;
      return ++lastStateID;
   }

   /// Generate a new "Pseudo Queue Set" id
   queueSet_t new_pseudo_queue_set();

   /// Delete a new "Pseudo Queue Set" id, and release all associated queues
   void release_pseudo_queue_set(queueSet_t queueSet);

   /// Get the allocations for each pseudo queue set
   void get_pseudo_queue_set_alloc(std::map<queueSet_t, double> &alloc);

   /// Set the allocations for each pseudo queue set
   void set_pseudo_queue_set_alloc(const std::map<queueSet_t, double> &alloc);


   /// Generate a new "Pseudo Queue" as a member of an existing queue set
   pseudoQueue_t new_pseudo_queue(queueSet_t queueSet);

   /// Generate a new "Pseudo Queue" as a member of an existing queue set
   void release_pseudo_queue(queueSet_t queueSet, pseudoQueue_t queue);

   /// Set the pseudo queue allocations within a queue set
   void get_pseudo_queue_alloc( queueSet_t queueSet, 
                                std::map<pseudoQueue_t, double> &alloc );

   /// Set the pseudo queue allocations within a queue set
   void set_pseudo_queue_alloc( queueSet_t queueSet, 
                                const std::map<pseudoQueue_t, double> &alloc );

   bool verbosity(int level) const
   { return ConcreteOptSolver_t::verbosity(level); }

protected:
   std::string define_solver_type() const
   { return "MultiState"; }

private:  // data types
   struct QueueSetInfo
   {
      double alloc;
      std::map<pseudoQueue_t, double> queues;
   };


private:  // methods
   void set_eval_mngr_allocations();

   ///
   virtual void reset_MultiStatePS();

private:  // member data
   MultiStatePatternConfig config;

   queueSet_t   m_lastQueueSet;

   std::map<queueSet_t, QueueSetInfo>   m_queueSets;

   std::map<colin::EvaluationID, StateContainer>   m_pending;
};



/// Base class for developing PS state representations and processing
/// functions
class MultiStatePS::PatternState 
{
   friend class MultiStatePS;
public:
   /// Constructor
   PatternState() 
      : step(0),
        center(),
        pending(0),
        config(NULL),
        state_id(PatternState::nextStateID())
   {}

   PatternState( const PatternState *old_state, response_t center_point )
      : step(0),
        center(center_point),
        pending(0),
        config(NULL),
        state_id(PatternState::nextStateID())
   {
      if ( old_state != NULL )
      {
         step = old_state->step;
         config = old_state->config;
      }
   }

   // Copy Constructor, Copy operator:
   //   -> no dynamic alloc, so not needed

   /// Destructor
   virtual ~PatternState() {}


   virtual long pattern_id() const = 0;

   long id() const
   { return state_id; }

   virtual void initialize_pattern( MultiStatePS *msps,
                                    response_t center_point )
   {
      assert( msps != NULL );

      config = &(msps->config);
      step   = config->delta_init;
      center = center_point;
   }

   virtual void process( response_t response, 
                         colin::EvaluationID eval_id ) = 0;

protected: // state data
   double       step;
   response_t   center;
   int          pending;

protected: // pattern data
   //MultiStatePS                   *msps;
   const MultiStatePatternConfig  *config;

private:
   static long nextStateID()
   {
      static long lastStateID = 0;
      return ++lastStateID;
   }

   long state_id;
};

} // namespace scolib

#endif // scolib_MultiStatePS_h
