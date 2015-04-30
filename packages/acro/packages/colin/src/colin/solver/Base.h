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
 * \file Solver_Base.h
 *
 * Defines the colin::Solver_Base class.
 */

#ifndef colin_Solver_Base_h
#define colin_Solver_Base_h

#include <acro_config.h>

#include <colin/EvaluationManager.h>
#include <colin/ObjectType.h>
#include <colin/PointSet.h>
#include <colin/StatusCodes.h>
#include <colin/Handle.h>

#include <utilib/Any.h>
//#include <utilib/OptionParser.h>
#include <utilib/AnyRNG.h>
#include <utilib/PropertyDict.h>

#include <boost/signals.hpp>
#include <boost/bind.hpp>
#ifdef ACRO_HAVE_MPI
#include <mpi.h>
#endif


class TiXmlElement;

namespace colin
{

class Solver_Base;
class SolverExecuteFunctor;

/// Define a handle to a (derived) Solver class
typedef Handle<Solver_Base> SolverHandle;


/// Structure containing information about the current status of a solver
struct SolverStatus {
   SolverStatus()
      : termination_info("unknown"),
        model_status(model_status_unknown),
        solver_status(solver_unknown),
        termination_condition(termination_unknown),
        termination_other("")
   {}

   utilib::PropertyDict describe(int verbosity) 
   {
      utilib::PropertyDict pd = utilib::PropertyDict(true);
      pd["status"] = solver_status;
      pd["termination condition"] = termination_condition;
      pd["termination message"] = termination_info;
      return pd;
   }

   std::string termination_info;
   model_status_enum model_status;
   solver_status_enum solver_status;
   termination_condition_enum termination_condition;
   std::string termination_other;
};


/**
 * Core class for all optimization solvers.
 * Upon termination from the \c minimization() method, the optimizer
 * contains a copy of the best solution, which can be retrieved with a
 * call to the \c best_point method.  The stream
 * operators for IO are defined to use the \c  write and \c read
 * methods, so subclasses of colin::Solver do not need to define these
 * stream operators.
 *
 */
class Solver_Base : public Handle_Client<Solver_Base>
{
public:
   enum PropertyIteratorLocation {
      BEGIN = 0,
      END = 1
   };

public: // Constructor methods

   /// Constructor
   Solver_Base();

   /// Virtual destructor
   virtual ~Solver_Base();

   /// Returns the solver type.
   /// TODO: document how/why this uses the evaluation manager
   std::string type() const;

   /// The processor for executing solvers from XML (defined in SolverMngr.cpp)
   friend class SolverExecuteFunctor;

public: // Methods to construct and configure an optimizer

   /// Set the problem
   virtual void set_problem(const ApplicationHandle handle) = 0;

   /// Construct solver from an XML fragment. Called to "set up" the solver.
   void construct(TiXmlElement* root, bool describe = false);

   /// Configure solver options
   //void configure(utilib::OptionParser &options);

   /// Retrieve the OptionsParser object that describes this solver's options
   //utilib::OptionParser& options()
   //{ return option; }

   /// True if the application has the named property
   bool has_property( std::string name )
   { return properties.exists(name); }

   /// get a reference to the named property
   utilib::Property& property( std::string name );

   /// get a reference to the named property
   utilib::Property& operator[]( std::string name )
   { return property(name); }

   /// Return a copy of the the underlying property dictionary
   utilib::PropertyDict Properties()
   { return properties; }

   /// Get the initial points cache
   PointSet get_initial_points()
   { return initial_points; }

   /// Specify the set of points the solver should initialize itself from
   void set_initial_points(PointSet ps);

   /// Insert the point into the initial PointSet (AppResponse or domain value)
   void add_initial_point(const utilib::AnyRef point);

   /// Sets the random number generator
   /** Set the random number generator that this solver should use.
    *  This API is being preserved from COLIN 2.0, but may change
    *  significantly in COLIN 3.1.
    *
    * \todo refactor and rework the use of RNGs within colin.  
    */
   template <class RNGT>
   void set_rng(RNGT* rng_)
   {
      if (rng_ && (rng != rng_))
      {
         rng = rng_;
      }
   }

public: // Methods to perform optimization and get the optimization results

   /// Reset this solver and prepare for the first/next call to optimize()
   void reset()
   { reset_signal(); }

   /// Perform the optimization.
   /** Perform the optimization.  The sense of optimization is defined
    *  by the Problem that is being optimized.
    */
   virtual void optimize() = 0;

   ///
   virtual void postsolve()
   {
      guess_solver_status( solver_status.solver_status,
                           solver_status.termination_condition );
   }

   /// Retrieve the PointSet of final solution(s) from the solver
   PointSet get_final_points() const;

   /// Retrieve the PointSet of optimal solution(s) from the solver
   //PointSet get_optimal_points() const;

   /// Return the current status of the solver and model (problem)
   const SolverStatus& status() const
   { return solver_status; }

   /// Return the current status of the solver and model (problem)
   const utilib::PropertyDict& statistics() const
   { return solver_statistics; }
    
   void set_statistic(std::string name, utilib::Any value);

   /// Return the results from the most-recent call to optimize()
   utilib::PropertyDict results(int verbosity = 0)
   {
      utilib::PropertyDict ans;
      ans.implicitDeclareIfDNE() = true;
      results_signal(ans, verbosity);
      return ans;
   }

public: // methods that are used by colin, but probably not by colin users

   /// Get a handle to the current problem
   virtual ApplicationHandle get_problem_handle() const = 0;

   ///
   void set_evaluation_manager(EvaluationManager_Handle new_manager)
   {
      m_eval_manager = EvaluationManager(new_manager);
      //std::cerr << "solver '" << typeid(*this).name()
      //          << "' assigned solver id = "
      //          << m_eval_manager.solverID() << std::endl;
   }

   ///
   const EvaluationManager& eval_mngr()
   {
      if (! m_eval_manager)
      {
         /** I want the Solver to try to grab a handle from the problem
          *  BEFORE defaulting to the main default manager.
          */
         try
         {
            m_eval_manager = get_problem_evaluation_manager();
         }
         catch (std::runtime_error&)
         {
            m_eval_manager = EvalManagerFactory().default_manager();
         }
         //std::cerr << "solver '" << typeid(*this).name()
         //          << "' inherited solver id = "
         //          << m_eval_manager.solverID() << std::endl;
      }
      return m_eval_manager;
   }

    ///
    int verbosity(const int level) const
        {return commonio.verbosity(level);}

protected: // methods

   /// Return the evaluation manager associated with this solver's 
   /// Problem instance.
   virtual const EvaluationManager& get_problem_evaluation_manager() = 0;

   /// The signal prototype for construct() element processing callbacks
   typedef boost::signal< void(TiXmlElement*, bool) >  ConstructSignal_t;

   /// Register derived class callback function for construct()
   ConstructSignal_t& register_construct(std::string element);

   /// Return the "type" of this solver as defined by the solver creator
   /** This method returns the solver "type" name as defined by the
    *  solver creator (or the creator of the solver wrapper).  Ideally,
    *  the solver should ALSO register itself with the SolverManager
    *  using the same string.
    */
   virtual std::string define_solver_type() const = 0;

   /// A convenience function that indicates the number of function
   /// evaluations that have been requested.  This returns zero if no
   /// problem application has been defined, to provide a consistent 
   /// semantics.
   /// WEH - it isn't clear that this belongs here, but this is used
   ///       frequently by subclasses of Solver.
   int neval() const
   {
      ApplicationHandle app = get_problem_handle();
      return ( app.empty() ? 0 : app->eval_count() );
   }

protected: // data

   /// The random number generator.
   utilib::AnyRNG rng;

   /// This solver's option parser
   //utilib::OptionParser option;

   /// All properties registered by derived classes
   utilib::Privileged_PropertyDict properties;

   /// The container for holding the initial points for this solver
   PointSet initial_points;

   /// The container for holding the final results for this solver
   PointSet final_points;

   /// The container for holding the optimal ("best") results for this solver
   PointSet optimal_points;

   /// Information about the solver status, and the associated final points.
   SolverStatus solver_status;

   ///
   utilib::PropertyDict solver_statistics;

   /// Called by reset() to reset derived classes before call to optimize()
   boost::signal<void()>               reset_signal;

   /// Called by results() to collect the solver results from derived classes
   boost::signal<void(utilib::PropertyDict&, int)>  results_signal;

   ///
   utilib::CommonIO commonio;

private: // methods
   ///
   void reset_Solver_Base();
   ///
   void cb_results(utilib::PropertyDict& pd, int verbosity);
   ///
   void process_xml_problem( TiXmlElement* node, bool describe );
   ///
   void process_xml_initialPoint( TiXmlElement* node, bool describe );
   ///
   void process_xml_finalPoint( TiXmlElement* node, bool describe );
   ///
   //void process_xml_optimalPoint( TiXmlElement* node, bool describe );
   ///
   void process_xml_options( TiXmlElement* node, bool describe );
   /// Set up initial point cache info obtained from XML by configure()
   void initialize_xml_init_cache();
   /// Set up final point cache info obtained from XML by configure()
   void initialize_xml_final_cache();
   /// Set up optimal point cache info obtained from XML by configure()
   void initialize_xml_optimal_cache();
private: // data

   ///
   struct Data;

   ///
   Data* data;

   ///
   EvaluationManager m_eval_manager;
};

} // namespace colin

#endif // colin_Solver_Base_h
