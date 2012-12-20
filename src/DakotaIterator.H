/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Iterator
//- Description:  Base class for iterator objects and envelope for 
//-               letter/envelope idiom.
//- Owner:        Mike Eldred
//- Version: $Id: DakotaIterator.H 7029 2010-10-22 00:17:02Z mseldre $

#ifndef DAKOTA_ITERATOR_H
#define DAKOTA_ITERATOR_H

#include "data_types.h"
#include "DakotaModel.H"


namespace Dakota {

class ProblemDescDB;
class Variables;
class Response;


/// Base class for the iterator class hierarchy.

/** The Iterator class is the base class for one of the primary class
    hierarchies in DAKOTA.  The iterator hierarchy contains all of the
    iterative algorithms which use repeated execution of simulations
    as function evaluations.  For memory efficiency and enhanced
    polymorphism, the iterator hierarchy employs the "letter/envelope
    idiom" (see Coplien "Advanced C++", p. 133), for which the base
    class (Iterator) serves as the envelope and one of the derived
    classes (selected in Iterator::get_iterator()) serves as the letter. */

class Iterator
{
public:

  //
  //- Heading: Constructors, destructor, assignment operator
  //

  /// default constructor
  Iterator();
  /// standard envelope constructor
  Iterator(Model& model);
  /// alternate envelope constructor for instantiations by name
  Iterator(const String& method_name, Model& model);
  /// copy constructor
  Iterator(const Iterator& iterator);

  /// destructor
  virtual ~Iterator();

  /// assignment operator
  Iterator operator=(const Iterator& iterator);

  //
  //- Heading: Virtual functions
  //

  // Run phase functions: derived classes reimplementing one of these
  // must call the same function in their closest parent which implements.

  /// utility function to perform common operations prior to pre_run();
  /// typically memory initialization; setting of instance pointers
  virtual void initialize_run();
  /// pre-run portion of run_iterator (optional); re-implemented by Iterators
  /// which can generate all Variables (parameter sets) a priori
  virtual void pre_run();
  /// run portion of run_iterator; implemented by all derived classes
  /// and may include pre/post steps in lieu of separate pre/post
  virtual void run();
  /// post-run portion of run_iterator (optional); verbose to print results;
  /// re-implemented by Iterators that can read all Variables/Responses and
  /// perform final analysis phase in a standalone way
  virtual void post_run(std::ostream& s);
  /// utility function to perform common operations following post_run();
  /// deallocation and resetting of instance pointers
  virtual void finalize_run();

  /// restore initial state for repeated sub-iterator executions
  virtual void reset();

  /// return a single final iterator solution (variables)
  virtual const Variables& variables_results() const;
  /// return a single final iterator solution (response)
  virtual const Response&  response_results() const;

  /// return multiple final iterator solutions (variables).  This should
  /// only be used if returns_multiple_points() returns true.
  virtual const VariablesArray& variables_array_results();
  /// return multiple final iterator solutions (response).  This should
  /// only be used if returns_multiple_points() returns true.
  virtual const ResponseArray&  response_array_results();

  /// indicates if this iterator accepts multiple initial points.  Default
  /// return is false.  Override to return true if appropriate.
  virtual bool accepts_multiple_points() const;
  /// indicates if this iterator returns multiple final points.  Default
  /// return is false.  Override to return true if appropriate.
  virtual bool returns_multiple_points() const;

  /// sets the multiple initial points for this iterator.  This should
  /// only be used if accepts_multiple_points() returns true.
  virtual void initial_points(const VariablesArray& pts);

  /// set the requested data for the final iterator response results
  virtual void response_results_active_set(const ActiveSet& set);

  /// initialize the 2D graphics window and the tabular graphics data
  virtual void initialize_graphics(bool graph_2d, bool tabular_data,
				   const String& tabular_file);

  /// print the final iterator results
  virtual void print_results(std::ostream& s);

  /// get the current number of samples
  virtual int num_samples() const;

  /// reset sampling iterator to use at least min_samples
  virtual void sampling_reset(int min_samples, bool all_data_flag, 
			      bool stats_flag);

  /// return sampling name
  virtual const String& sampling_scheme() const;

  /// return the result of any recasting or surrogate model recursion
  /// layered on top of iteratedModel by the derived Iterator ctor chain
  virtual const Model& algorithm_space_model() const;

  /// return name of any enabling iterator used by this iterator
  virtual String uses_method() const;
  /// perform a method switch, if possible, due to a detected conflict
  virtual void method_recourse();

  /// return the complete set of evaluated variables
  virtual const VariablesArray& all_variables();
  /// return the complete set of evaluated samples
  virtual const RealMatrix&     all_samples();
  /// return the complete set of computed responses
  virtual const IntResponseMap& all_responses() const;

  /// returns Analyzer::compactMode
  virtual bool compact_mode() const;

  //
  //- Heading: Member functions
  //

  /// orchestrate initialize/pre/run/post/finalize phases
  void run_iterator(std::ostream& s);

  /// replaces existing letter with a new one
  void assign_rep(Iterator* iterator_rep, bool ref_count_incr = true);

  // set the model
  //void iterated_model(const Model& the_model);
  // return the model
  //Model& iterated_model() const;

  /// return the problem description database (probDescDB)
  ProblemDescDB& problem_description_db() const;

  /// return the method name
  const String& method_name() const;
  /// return the method identifier (methodId)
  const String& method_id() const;

  /// set the method convergence tolerance (convergenceTol)
  void convergence_tolerance(Real conv_tol);
  /// return the method convergence tolerance (convergenceTol)
  Real convergence_tolerance() const;

  /// set the method output level (outputLevel)
  void output_level(short out_lev);
  /// return the method output level (outputLevel)
  short output_level() const;
  /// Set summary output control; true enables evaluation/results summary
  void summary_output(bool summary_output_flag);

  /// return the maximum concurrency supported by the iterator
  int maximum_concurrency() const;
  /// set the maximum concurrency supported by the iterator
  void maximum_concurrency(int max_conc);

  /// return the number of solutions to retain in best variables/response arrays
  size_t num_final_solutions() const;
  /// set the number of solutions to retain in best variables/response arrays
  void num_final_solutions(size_t num_final);

  /// set the default active set vector (for use with iterators that
  /// employ evaluate_parameter_sets())
  void active_set(const ActiveSet& set);
  /// return the default active set vector (used by iterators that
  /// employ evaluate_parameter_sets())
  const ActiveSet& active_set() const;

  /// set subIteratorFlag (and update summaryOutputFlag if needed)
  void sub_iterator_flag(bool si_flag);
  /// set primaryA{CV,DIV,DRV}MapIndices, secondaryA{CV,DIV,DRV}MapTargets
  void active_variable_mappings(const SizetArray& c_index1,
				const SizetArray& di_index1,
				const SizetArray& dr_index1,
				const ShortArray& c_target2,
				const ShortArray& di_target2,
				const ShortArray& dr_target2);

  /// function to check iteratorRep (does this envelope contain a letter?)
  bool is_null() const;

  /// returns iteratorRep for access to derived class member functions
  /// that are not mapped to the top Iterator level
  Iterator* iterator_rep() const;

  //
  //- Heading: Operator overloaded functions
  //

  // Possible future implementation for enhanced granularity in
  // Iterator virtual function.  Could be very useful for Strategy
  // level control!
  //virtual void operator++();  // increment operator
  //virtual void operator--();  // decrement operator

protected:

  //
  //- Heading: Constructors
  //

  /// constructor initializes the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  Iterator(BaseConstructor, Model& model);

  /// alternate constructor for base iterator classes constructed on the fly
  Iterator(NoDBBaseConstructor, Model& model);

  /// alternate constructor for base iterator classes constructed on the fly
  Iterator(NoDBBaseConstructor);

  //
  //- Heading: Virtual functions
  //

  /// gets the multiple initial points for this iterator.  This will only
  /// be meaningful after a call to initial_points mutator.
  virtual const VariablesArray& initial_points() const;

  //
  //- Heading: Data
  //

  /// shallow copy of the model passed into the constructor
  /// or a thin RecastModel wrapped around it
  Model iteratedModel;

  /// class member reference to the problem description database
  ProblemDescDB& probDescDB;

  String methodName; ///< name of the iterator (the user's method spec)

  Real convergenceTol;  ///< iteration convergence tolerance
  int maxIterations;    ///< maximum number of iterations for the iterator
  int maxFunctionEvals; ///< maximum number of fn evaluations for the iterator
  int maxConcurrency;   ///< maximum coarse-grained concurrency

  // Isolate complexity by letting Model::currentVariables/currentResponse
  // manage details.  Then Iterator only needs the following:
  size_t numFunctions;        ///< number of response functions
  size_t numContinuousVars;   ///< number of active continuous vars
  size_t numDiscreteIntVars;  ///< number of active discrete integer vars
  size_t numDiscreteRealVars; ///< number of active discrete real vars

  ///  number of solutions to retain in best variables/response arrays
  size_t numFinalSolutions;

  /// tracks the response data requirements on each function evaluation
  ActiveSet activeSet;

  /// collection of N best solution variables found during the study
  VariablesArray bestVariablesArray;
  /// collection of N best solution responses found during the study
  ResponseArray bestResponseArray;

  /// flag indicating if this Iterator is a sub-iterator
  /// (NestedModel::subIterator or DataFitSurrModel::daceIterator)
  bool subIteratorFlag;
  /// "primary" all continuous variable mapping indices flowed down
  /// from higher level iteration
  SizetArray primaryACVarMapIndices;
  /// "primary" all discrete int variable mapping indices flowed down from
  /// higher level iteration
  SizetArray primaryADIVarMapIndices;
  /// "primary" all discrete real variable mapping indices flowed down from
  /// higher level iteration
  SizetArray primaryADRVarMapIndices;
  /// "secondary" all continuous variable mapping targets flowed down
  /// from higher level iteration
  ShortArray secondaryACVarMapTargets;
  /// "secondary" all discrete int variable mapping targets flowed down
  /// from higher level iteration
  ShortArray secondaryADIVarMapTargets;
  /// "secondary" all discrete real variable mapping targets flowed down
  /// from higher level iteration
  ShortArray secondaryADRVarMapTargets;

  /// type of gradient data: analytic, numerical, mixed, or none
  String gradientType;
  /// source of numerical gradient routine: dakota or vendor
  String methodSource;
  /// type of numerical gradient interval: central or forward
  String intervalType;
  /// type of Hessian data: analytic, numerical, quasi, mixed, or none
  String hessianType;

  /// relative finite difference step size for numerical gradients
  /** A scalar value (instead of the vector fd_gradient_step_size spec) is
      used within the iterator hierarchy since this attribute is only used
      to publish a step size to vendor numerical gradient algorithms. */
  Real fdGradStepSize;
  /// type of finite difference step to use for numerical gradient:
  /// relative - step length is relative to x
  /// absolute - step length is what is specified
  /// bounds - step length is relative to range of x
  String fdGradStepType;
  /// relative finite difference step size for numerical Hessians estimated 
  /// using first-order differences of gradients
  /** A scalar value (instead of the vector fd_hessian_step_size spec) is
      used within the iterator hierarchy since this attribute is only used
      to publish a step size to vendor numerical Hessian algorithms. */
  Real fdHessByGradStepSize;
  /// relative finite difference step size for numerical Hessians estimated 
  /// using second-order differences of function values
  /** A scalar value (instead of the vector fd_hessian_step_size spec) is
      used within the iterator hierarchy since this attribute is only used
      to publish a step size to vendor numerical Hessian algorithms. */
  Real fdHessByFnStepSize;
  /// type of finite difference step to use for numerical Hessian:
  /// relative - step length is relative to x
  /// absolute - step length is what is specified
  /// bounds - step length is relative to range of x
  String fdHessStepType;

  /// output verbosity level: {SILENT,QUIET,NORMAL,VERBOSE,DEBUG}_OUTPUT
  short outputLevel;
  /// flag for summary output (evaluation stats, final results);
  /// default true, but false for on-the-fly (helper) iterators and
  /// sub-iterator use cases
  bool summaryOutputFlag;

  /// copy of the model's asynchronous evaluation flag
  bool asynchFlag;

  /// write precision as specified by the user
  int writePrecision;

private:

  //
  //- Heading: Member functions
  //

  /// Used by the envelope to instantiate the correct letter class
  Iterator* get_iterator(Model& model);
  /// Used by the envelope to instantiate the correct letter class
  Iterator* get_iterator(const String& method_name, Model& model);

  /// convenience function to write variables to file, following pre-run
  virtual void pre_output();

  /// read tabular data for post-run mode
  virtual void post_input();
  
  //
  //- Heading: Data
  //

  /// method identifier string from the input file
  String methodId;
  /// pointer to the letter (initialized only for the envelope)
  Iterator* iteratorRep;
  /// number of objects sharing iteratorRep
  int referenceCount;
};


// nonvirtual functions can access letter attributes directly (only need to fwd
// member function call when the function could be redefined).
//inline Model& Iterator::iterated_model() const
//{ return (iteratorRep) ? iteratorRep->iteratedModel : iteratedModel; }


//inline void Iterator::iterated_model(const Model& the_model)
//{
//  if (iteratorRep)
//    iteratorRep->iteratedModel = the_model;
//  else
//    iteratedModel = the_model;
//}


inline ProblemDescDB& Iterator::problem_description_db() const
{ return (iteratorRep) ? iteratorRep->probDescDB : probDescDB; }


inline const String& Iterator::method_name() const
{ return (iteratorRep) ? iteratorRep->methodName : methodName; }


inline const String& Iterator::method_id() const
{ return (iteratorRep) ? iteratorRep->methodId : methodId; }


inline void Iterator::convergence_tolerance(Real conv_tol)
{
  if (iteratorRep) iteratorRep->convergenceTol = conv_tol;
  else             convergenceTol = conv_tol; 
}


inline Real Iterator::convergence_tolerance() const
{ return (iteratorRep) ? iteratorRep->convergenceTol : convergenceTol; }


inline void Iterator::output_level(short out_lev)
{
  if (iteratorRep) iteratorRep->outputLevel = out_lev;
  else             outputLevel = out_lev; 
}


inline short Iterator::output_level() const
{ return (iteratorRep) ? iteratorRep->outputLevel : outputLevel; }


inline void Iterator::summary_output(bool summary_output_flag)
{ 
  if (iteratorRep) iteratorRep->summaryOutputFlag = summary_output_flag;
  else             summaryOutputFlag = summary_output_flag; 
}


inline int Iterator::maximum_concurrency() const
{ return (iteratorRep) ? iteratorRep->maxConcurrency : maxConcurrency; }


inline void Iterator::maximum_concurrency(int max_conc)
{
  if (iteratorRep) iteratorRep->maxConcurrency = max_conc;
  else             maxConcurrency = max_conc;
}


inline size_t Iterator::num_final_solutions() const
{ return (iteratorRep) ? iteratorRep->numFinalSolutions : numFinalSolutions; }


inline void Iterator::num_final_solutions(size_t num_final)
{ 
  if (iteratorRep) iteratorRep->numFinalSolutions = num_final;
  else             numFinalSolutions = num_final;
}


inline void Iterator::active_set(const ActiveSet& set)
{
  if (iteratorRep) iteratorRep->activeSet = set;
  else             activeSet = set;
}


inline const ActiveSet& Iterator::active_set() const
{ return (iteratorRep) ? iteratorRep->activeSet : activeSet; }


inline void Iterator::
active_variable_mappings(const SizetArray& c_index1,
			 const SizetArray& di_index1,
			 const SizetArray& dr_index1,
			 const ShortArray& c_target2,
			 const ShortArray& di_target2,
			 const ShortArray& dr_target2)
{
  if (iteratorRep) {
    iteratorRep->primaryACVarMapIndices    = c_index1;
    iteratorRep->primaryADIVarMapIndices   = di_index1;
    iteratorRep->primaryADRVarMapIndices   = dr_index1;
    iteratorRep->secondaryACVarMapTargets  = c_target2;
    iteratorRep->secondaryADIVarMapTargets = di_target2;
    iteratorRep->secondaryADRVarMapTargets = dr_target2;
  }
  else {
    primaryACVarMapIndices    = c_index1;
    primaryADIVarMapIndices   = di_index1;
    primaryADRVarMapIndices   = dr_index1;
    secondaryACVarMapTargets  = c_target2;
    secondaryADIVarMapTargets = di_target2;
    secondaryADRVarMapTargets = dr_target2;
  }
}


inline bool Iterator::is_null() const
{ return (iteratorRep) ? false : true; }


inline Iterator* Iterator::iterator_rep() const
{ return iteratorRep; }


/// global comparison function for Iterator
inline bool method_id_compare(const Iterator& iterator, const void* id)
{ return ( *(const String*)id == iterator.method_id() ); }

} // namespace Dakota

#endif
