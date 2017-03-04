/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Iterator
//- Description:  Base class for iterator objects and envelope for 
//-               letter/envelope idiom.
//- Owner:        Mike Eldred
//- Version: $Id: DakotaIterator.hpp 7029 2010-10-22 00:17:02Z mseldre $

#ifndef DAKOTA_ITERATOR_H
#define DAKOTA_ITERATOR_H

#include "dakota_data_types.hpp"
#include "DakotaModel.hpp"
#include "ResultsManager.hpp"


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
  /// alternate envelope constructor that assigns a representation pointer
  Iterator(Iterator* iterator_rep, bool ref_count_incr = true);
  /// standard envelope constructor, which constructs its own model(s)
  Iterator(ProblemDescDB& problem_db);
  /// alternate envelope constructor which uses the ProblemDescDB but
  /// accepts a model from a higher level (meta-iterator) context,
  /// instead of constructing its own
  Iterator(ProblemDescDB& problem_db, Model& model);
  /// alternate envelope constructor for instantiations by name
  /// without the ProblemDescDB
  Iterator(const String& method_string, Model& model);
  /// copy constructor
  Iterator(const Iterator& iterator);

  /// destructor
  virtual ~Iterator();

  /// assignment operator
  Iterator operator=(const Iterator& iterator);

  //
  //- Heading: Virtual functions
  //

  /// derived class contributions to setting the communicators
  /// associated with this Iterator instance
  virtual void derived_set_communicators(ParLevLIter pl_iter);
  /// derived class contributions to freeing the communicators
  /// associated with this Iterator instance
  virtual void derived_free_communicators(ParLevLIter pl_iter);

  // Run phase functions: derived classes reimplementing one of these
  // must call the same function in their closest parent which implements.

  /// utility function to perform common operations prior to pre_run();
  /// typically memory initialization; setting of instance pointers
  virtual void initialize_run();
  /// pre-run portion of run (optional); re-implemented by Iterators
  /// which can generate all Variables (parameter sets) a priori
  virtual void pre_run();
  /// core portion of run; implemented by all derived classes
  /// and may include pre/post steps in lieu of separate pre/post
  virtual void core_run();
  /// post-run portion of run (optional); verbose to print results;
  /// re-implemented by Iterators that can read all Variables/Responses and
  /// perform final analysis phase in a standalone way
  virtual void post_run(std::ostream& s);
  /// utility function to perform common operations following post_run();
  /// deallocation and resetting of instance pointers
  virtual void finalize_run();

  /// write variables to file, following pre-run
  virtual void pre_output();
  /// read tabular data for post-run mode
  virtual void post_input();
  
  /// restore initial state for repeated sub-iterator executions
  virtual void reset();

  /// used by IteratorScheduler to set the starting data for a run
  virtual void initialize_iterator(int job_index);
  /// used by IteratorScheduler to pack starting data for an iterator run
  virtual void pack_parameters_buffer(MPIPackBuffer& send_buffer,
				      int job_index);
  /// used by IteratorScheduler to unpack starting data for an iterator run
  virtual void unpack_parameters_buffer(MPIUnpackBuffer& recv_buffer,
					int job_index);
  /// used by IteratorScheduler to unpack starting data and initialize
  /// an iterator run
  virtual void unpack_parameters_initialize(MPIUnpackBuffer& recv_buffer,
					    int job_index);
  /// used by IteratorScheduler to pack results data from an iterator run
  virtual void pack_results_buffer(MPIPackBuffer& send_buffer, int job_index);
  /// used by IteratorScheduler to unpack results data from an iterator run
  virtual void unpack_results_buffer(MPIUnpackBuffer& recv_buffer,
				     int job_index);
  /// used by IteratorScheduler to update local results arrays
  virtual void update_local_results(int job_index);

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

  /// set the requested data for the final iterator response results
  virtual void response_results_active_set(const ActiveSet& set);

  /// return error estimates associated with the final iterator solution
  virtual const RealVector& response_error_estimates() const;

  /// indicates if this iterator accepts multiple initial points.  Default
  /// return is false.  Override to return true if appropriate.
  virtual bool accepts_multiple_points() const;
  /// indicates if this iterator returns multiple final points.  Default
  /// return is false.  Override to return true if appropriate.
  virtual bool returns_multiple_points() const;

  /// sets the multiple initial points for this iterator.  This should
  /// only be used if accepts_multiple_points() returns true.
  virtual void initial_points(const VariablesArray& pts);

  /// initialize the 2D graphics window and the tabular graphics data
  virtual void initialize_graphics(int iterator_server_id = 1);

  /// print the final iterator results
  virtual void print_results(std::ostream& s);

  /// return the result of any recasting or surrogate model recursion
  /// layered on top of iteratedModel by the derived Iterator ctor chain
  virtual const Model& algorithm_space_model() const;

  /// return name of any enabling iterator used by this iterator
  virtual unsigned short uses_method() const;
  /// perform a method switch, if possible, due to a detected conflict
  virtual void method_recourse();

  /// return the complete set of evaluated variables
  virtual const VariablesArray& all_variables();
  /// return the complete set of evaluated samples
  virtual const RealMatrix&     all_samples();
  /// return the complete set of computed responses
  virtual const IntResponseMap& all_responses() const;

  /// get the current number of samples
  virtual int num_samples() const;
  /// reset sampling iterator to use at least min_samples
  virtual void sampling_reset(int min_samples, bool all_data_flag, 
			      bool stats_flag);
  /// set reference number of samples, which is a lower bound during reset 
  virtual void sampling_reference(int samples_ref);

  /// increment to next in sequence of refinement samples
  virtual void sampling_increment();

  /// return sampling name
  virtual unsigned short sampling_scheme() const;

  /// returns Analyzer::compactMode
  virtual bool compact_mode() const;

  /// estimate the minimum and maximum partition sizes that can be
  /// utilized by this Iterator
  virtual IntIntPair estimate_partition_bounds();

  /// reinitializes iterator based on new variable size
  virtual bool resize();

  //
  //- Heading: Member functions
  //

  /// initialize the communicators associated with this Iterator instance
  void init_communicators(ParLevLIter pl_iter);
  /// set the communicators associated with this Iterator instance
  void set_communicators(ParLevLIter pl_iter);
  /// free the communicators associated with this Iterator instance
  void free_communicators(ParLevLIter pl_iter);

  /// Resize the communicators. This is called from the letter's resize()
  void resize_communicators(ParLevLIter pl_iter, bool reinit_comms);

  /// set methodPCIter
  void parallel_configuration_iterator(ParConfigLIter pc_iter);
  /// return methodPCIter
  ParConfigLIter parallel_configuration_iterator() const;

  /// invoke set_communicators(pl_iter) prior to run()
  void run(ParLevLIter pl_iter);
  /// orchestrate initialize/pre/core/post/finalize phases
  void run();

  /// replaces existing letter with a new one
  void assign_rep(Iterator* iterator_rep, bool ref_count_incr = true);

  /// set the iteratedModel (iterators and meta-iterators using a single
  /// model instance)
  void iterated_model(const Model& model);
  /// return the iteratedModel (iterators & meta-iterators using a single
  /// model instance)
  Model& iterated_model();

  /// return the problem description database (probDescDB)
  ProblemDescDB& problem_description_db() const;
  /// return the parallel library (parallelLib)
  ParallelLibrary& parallel_library() const;

  /// set the method name to an enumeration value
  void method_name(unsigned short m_name);
  /// return the method name via its native enumeration value
  unsigned short method_name() const;
  /// set the method name by string
  void method_string(const String& m_str);
  /// return the method name by string
  String method_string() const;

  /// convert a method name enumeration value to a string
  String method_enum_to_string(unsigned short method_name) const;
  /// convert a method name string to an enumeration value
  unsigned short method_string_to_enum(const String& method_name) const;
  /// convert a method name enumeration value to a string
  String submethod_enum_to_string(unsigned short submethod_name) const;

  /// return the method identifier (methodId)
  const String& method_id() const;

  /// return the maximum evaluation concurrency supported by the iterator
  int maximum_evaluation_concurrency() const;
  /// set the maximum evaluation concurrency supported by the iterator
  void maximum_evaluation_concurrency(int max_conc);

  /// return the maximum iterations for this iterator
  int maximum_iterations() const;
  /// set the maximum iterations for this iterator
  void maximum_iterations(int max_iter);

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
				const SizetArray& ds_index1,
				const SizetArray& dr_index1,
				const ShortArray& c_target2,
				const ShortArray& di_target2,
				const ShortArray& ds_target2,
				const ShortArray& dr_target2);

  /// function to check iteratorRep (does this envelope contain a letter?)
  bool is_null() const;

  /// returns iteratorRep for access to derived class member functions
  /// that are not mapped to the top Iterator level
  Iterator* iterator_rep() const;

  /// set the hierarchical eval ID tag prefix
  virtual void eval_tag_prefix(const String& eval_id_str);

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
  Iterator(BaseConstructor, ProblemDescDB& problem_db);

  /// alternate constructor for base iterator classes constructed on the fly
  Iterator(NoDBBaseConstructor, unsigned short method_name, Model& model);

  /// alternate constructor for base iterator classes constructed on the fly
  Iterator(NoDBBaseConstructor, unsigned short method_name);

  //
  //- Heading: Virtual functions
  //

  /// derived class contributions to initializing the communicators
  /// associated with this Iterator instance
  virtual void derived_init_communicators(ParLevLIter pl_iter);

  /// set inherited data attributes based on extractions from incoming model
  virtual void update_from_model(const Model& model);

  /// gets the multiple initial points for this iterator.  This will only
  /// be meaningful after a call to initial_points mutator.
  virtual const VariablesArray& initial_points() const;

  //
  //- Heading: Member functions
  //

  /// get the unique run identifier based on method name, id, and
  /// number of executions
  StrStrSizet run_identifier() const;

  /// conversion of request vector values for the Gauss-Newton Hessian
  /// approximation
  static void gnewton_set_recast(const Variables& recast_vars,
				 const ActiveSet& recast_set,
				 ActiveSet& sub_model_set);

  //
  //- Heading: Data
  //

  /// class member reference to the problem description database
  /** Iterator and Model cannot use a shallow copy of ProblemDescDB
      due to circular destruction dependency (reference counts can't
      get to 0), since ProblemDescDB contains {iterator,model}List. */
  ProblemDescDB& probDescDB;

  /// class member reference to the parallel library
  ParallelLibrary& parallelLib;

  /// the active ParallelConfiguration used by this Iterator instance
  ParConfigLIter methodPCIter;
  // index for the active ParallelLevel within ParallelConfiguration::miPLIters
  //size_t miPLIndex;

  /// the model to be iterated (for iterators and meta-iterators
  /// employing a single model instance)
  Model iteratedModel;

  /// number of Models locally (in Iterator or derived classes)
  /// wrapped around the initially passed in Model
  size_t myModelLayers;

  unsigned short methodName; ///< name of the iterator (the user's method spec)

  Real convergenceTol;  ///< iteration convergence tolerance
  int maxIterations;    ///< maximum number of iterations for the iterator
  int maxFunctionEvals; ///< maximum number of fn evaluations for the iterator

  /// maximum number of concurrent model evaluations
  /** This is important for parallel configuration init/set/free and may be
      set within empty envelope instances.  Therefore, it cannot be pushed
      down into Analyzer/Minimizer derived classes. */
  int maxEvalConcurrency;

  /// the response data requirements on each function evaluation
  ActiveSet activeSet;

  /// number of solutions to retain in best variables/response arrays
  size_t numFinalSolutions;
  /// collection of N best solution variables found during the study;
  /// always in context of Model originally passed to the Iterator
  /// (any in-flight Recasts must be undone)
  VariablesArray bestVariablesArray;
  /// collection of N best solution responses found during the study;
  /// always in context of Model originally passed to the Iterator
  /// (any in-flight Recasts must be undone)
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
  /// "primary" all discrete string variable mapping indices flowed down from
  /// higher level iteration
  SizetArray primaryADSVarMapIndices;
  /// "primary" all discrete real variable mapping indices flowed down from
  /// higher level iteration
  SizetArray primaryADRVarMapIndices;
  /// "secondary" all continuous variable mapping targets flowed down
  /// from higher level iteration
  ShortArray secondaryACVarMapTargets;
  /// "secondary" all discrete int variable mapping targets flowed down
  /// from higher level iteration
  ShortArray secondaryADIVarMapTargets;
  /// "secondary" all discrete string variable mapping targets flowed down
  /// from higher level iteration
  ShortArray secondaryADSVarMapTargets;
  /// "secondary" all discrete real variable mapping targets flowed down
  /// from higher level iteration
  ShortArray secondaryADRVarMapTargets;

  /// output verbosity level: {SILENT,QUIET,NORMAL,VERBOSE,DEBUG}_OUTPUT
  short outputLevel;
  /// flag for summary output (evaluation stats, final results); default true,
  /// but false for on-the-fly (helper) iterators and sub-iterator use cases
  bool summaryOutputFlag;

  /// reference to the global iterator results database
  ResultsManager& resultsDB;
  /// valid names for iterator results
  ResultsNames resultsNames;

private:

  //
  //- Heading: Member functions
  //

  /// Used by the envelope to instantiate the correct letter class
  Iterator* get_iterator(ProblemDescDB& problem_db);
  /// Used by the envelope to instantiate the correct letter class
  Iterator* get_iterator(ProblemDescDB& problem_db, Model& model);
  /// Used by the envelope to instantiate the correct letter class
  Iterator* get_iterator(const String& method_string, Model& model);

  //
  //- Heading: Data
  //

  /// method identifier string from the input file
  String methodId;

  /// an execution number for this instance of the class, unique
  /// across all instances of same methodName/methodId
  size_t execNum;

  /// track the available configurations that have been created
  /// (init_communicators) and are available for activation at run
  /// time (set_communicators)
  std::map<size_t, ParConfigLIter> methodPCIterMap;

  /// pointer to the letter (initialized only for the envelope)
  Iterator* iteratorRep;
  /// number of objects sharing iteratorRep
  int referenceCount;
};


inline void Iterator::parallel_configuration_iterator(ParConfigLIter pc_iter)
{
  if (iteratorRep) iteratorRep->methodPCIter = pc_iter;
  else             methodPCIter = pc_iter;
}


inline ParConfigLIter Iterator::parallel_configuration_iterator() const
{ return (iteratorRep) ? iteratorRep->methodPCIter : methodPCIter; }


inline void Iterator::iterated_model(const Model& model)
{
  if (iteratorRep) iteratorRep->iteratedModel = model;
  else             iteratedModel = model; 
}


inline Model& Iterator::iterated_model()
{ return (iteratorRep) ? iteratorRep->iteratedModel : iteratedModel; }


inline ProblemDescDB& Iterator::problem_description_db() const
{ return (iteratorRep) ? iteratorRep->probDescDB : probDescDB; }


inline ParallelLibrary& Iterator::parallel_library() const
{ return (iteratorRep) ? iteratorRep->parallelLib : parallelLib; }


inline void Iterator::method_name(unsigned short m_name)
{
  if (iteratorRep) iteratorRep->methodName = m_name;
  else             methodName = m_name;
}


inline unsigned short Iterator::method_name() const
{ return (iteratorRep) ? iteratorRep->methodName : methodName; }


inline void Iterator::method_string(const String& m_str)
{
  if (iteratorRep) iteratorRep->methodName = method_string_to_enum(m_str);
  else             methodName = method_string_to_enum(m_str);
}


inline String Iterator::method_string() const
{
  return (iteratorRep) ? iteratorRep->method_string() :
    method_enum_to_string(methodName);
}


inline const String& Iterator::method_id() const
{ return (iteratorRep) ? iteratorRep->methodId : methodId; }


inline int Iterator::maximum_evaluation_concurrency() const
{ return (iteratorRep) ? iteratorRep->maxEvalConcurrency : maxEvalConcurrency; }


inline void Iterator::maximum_evaluation_concurrency(int max_conc)
{
  if (iteratorRep) iteratorRep->maxEvalConcurrency = max_conc;
  else             maxEvalConcurrency = max_conc;
}

inline int Iterator::maximum_iterations() const
{ return (iteratorRep) ? iteratorRep->maxIterations : maxIterations; }

inline void Iterator::maximum_iterations(int max_iter)
{
  if (iteratorRep) iteratorRep->maxIterations = max_iter;
  else             maxIterations = max_iter;
}

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
			 const SizetArray& ds_index1,
			 const SizetArray& dr_index1,
			 const ShortArray& c_target2,
			 const ShortArray& di_target2,
			 const ShortArray& ds_target2,
			 const ShortArray& dr_target2)
{
  if (iteratorRep) {
    iteratorRep->primaryACVarMapIndices    = c_index1;
    iteratorRep->primaryADIVarMapIndices   = di_index1;
    iteratorRep->primaryADSVarMapIndices   = ds_index1;
    iteratorRep->primaryADRVarMapIndices   = dr_index1;
    iteratorRep->secondaryACVarMapTargets  = c_target2;
    iteratorRep->secondaryADIVarMapTargets = di_target2;
    iteratorRep->secondaryADSVarMapTargets = ds_target2;
    iteratorRep->secondaryADRVarMapTargets = dr_target2;
  }
  else {
    primaryACVarMapIndices    = c_index1;
    primaryADIVarMapIndices   = di_index1;
    primaryADSVarMapIndices   = ds_index1;
    primaryADRVarMapIndices   = dr_index1;
    secondaryACVarMapTargets  = c_target2;
    secondaryADIVarMapTargets = di_target2;
    secondaryADSVarMapTargets = ds_target2;
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
