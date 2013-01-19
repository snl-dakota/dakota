/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Interface
//- Description:  Abstract base class for the interfaces used to map variables
//-               into responses.
//- Owner:        Michael Eldred
//- Version: $Id: DakotaInterface.hpp 7034 2010-10-22 20:16:32Z mseldre $

#ifndef DAKOTA_INTERFACE_H
#define DAKOTA_INTERFACE_H

#include "system_defs.hpp"
#include "global_defs.hpp" // for BaseConstructor
#include "data_types.hpp"
#include "DakotaString.hpp"

#ifdef HAVE_AMPL
struct ASL; // forward declaration
#endif // HAVE_AMPL

// likely need more general _WIN32 here
#ifdef __MINGW32__
#undef interface
#endif

// forward declaration
namespace Pecos { class SurrogateData; }

namespace Dakota {

// forward declarations
class ProblemDescDB;
class Variables;
class ActiveSet;
class Response;
class Iterator;
class Model;
class Approximation;
class AnalysisCode;


/// Base class for the interface class hierarchy.

/** The Interface class hierarchy provides the part of a Model that is
    responsible for mapping a set of Variables into a set of Responses.
    The mapping is performed using either a simulation-based application
    interface or a surrogate-based approximation interface.  For memory
    efficiency and enhanced polymorphism, the interface hierarchy
    employs the "letter/envelope idiom" (see Coplien "Advanced C++",
    p. 133), for which the base class (Interface) serves as the envelope
    and one of the derived classes (selected in Interface::get_interface())
    serves as the letter. */

class Interface
{
public:

  //
  //- Heading: Constructors, destructor, assignment operator
  //

  /// default constructor
  Interface();
  /// standard constructor for envelope
  Interface(ProblemDescDB& problem_db);
  /// copy constructor
  Interface(const Interface& interface);

  /// destructor
  virtual ~Interface();

  /// assignment operator
  Interface operator=(const Interface& interface);

  //
  //- Heading: Virtual functions
  //

  /// the function evaluator: provides a "mapping" from the
  /// variables to the responses.
  virtual void map(const Variables& vars, const ActiveSet& set,
		   Response& response, const bool asynch_flag = false);

  /// recovers data from a series of asynchronous evaluations (blocking)
  virtual const IntResponseMap& synch(); 

  /// recovers data from a series of asynchronous evaluations (nonblocking)
  virtual const IntResponseMap& synch_nowait(); 

  /// evaluation server function for multiprocessor executions
  virtual void serve_evaluations();

  /// send messages from iterator rank 0 to terminate evaluation servers
  virtual void stop_evaluation_servers();

  /// allocate communicator partitions for concurrent evaluations within an
  /// iterator and concurrent multiprocessor analyses within an evaluation.
  virtual void init_communicators(const IntArray& message_lengths, 
				  int max_iterator_concurrency);

  /// set the local parallel partition data for an interface
  /// (the partitions are already allocated in ParallelLibrary).
  virtual void set_communicators(const IntArray& message_lengths,
				 int max_iterator_concurrency);

  /// deallocate communicator partitions for concurrent evaluations within an
  /// iterator and concurrent multiprocessor analyses within an evaluation.
  virtual void free_communicators();

  /// reset certain defaults for serial interface objects.
  virtual void init_serial();

  /// return the user-specified concurrency for asynch local evaluations
  virtual int asynch_local_evaluation_concurrency() const;

  /// return the user-specified interface synchronization
  virtual String interface_synchronization() const;

  /// returns the minimum number of points required to build a particular
  /// ApproximationInterface (used by DataFitSurrModels).
  virtual int minimum_points(bool constraint_flag) const;

  /// returns the recommended number of points required to build a particular
  /// ApproximationInterface (used by DataFitSurrModels).
  virtual int recommended_points(bool constraint_flag) const;

  /// set the (currently active) approximation function index set
  virtual void approximation_function_indices(const IntSet& approx_fn_indices);

  /// updates the anchor point for an approximation
  virtual void update_approximation(const Variables& vars,
				    const IntResponsePair& response_pr);
  /// updates the current data points for an approximation
  virtual void update_approximation(const RealMatrix& samples,
				    const IntResponseMap& resp_map);
  /// updates the current data points for an approximation
  virtual void update_approximation(const VariablesArray& vars_array,
				    const IntResponseMap& resp_map);

  /// appends a single point to an existing approximation
  virtual void append_approximation(const Variables& vars,
				    const IntResponsePair& response_pr);
  /// appends multiple points to an existing approximation
  virtual void append_approximation(const RealMatrix& samples,
				    const IntResponseMap& resp_map);
  /// appends multiple points to an existing approximation
  virtual void append_approximation(const VariablesArray& vars_array,
				    const IntResponseMap& resp_map);

  /// builds the approximation
  virtual void build_approximation(const RealVector& c_l_bnds,
    const RealVector&  c_u_bnds, const IntVector&  di_l_bnds,
    const IntVector&  di_u_bnds, const RealVector& dr_l_bnds,
    const RealVector& dr_u_bnds);

  /// rebuilds the approximation after a data update
  virtual void rebuild_approximation(const BoolDeque& rebuild_deque);

  /// removes data from last append from the approximation
  virtual void pop_approximation(bool save_surr_data);

  /// restores the approximation to a selected previous state
  virtual void restore_approximation();
  /// queries the approximation for the ability to restore a previous increment
  virtual bool restore_available();

  /// finalizes the approximation by applying all trial increments
  virtual void finalize_approximation();

  /// move the current approximation into storage for later combination
  virtual void store_approximation();
  /// combine the current approximation with one previously stored
  virtual void combine_approximation(short corr_type);

  /// clears current data from an approximation interface
  virtual void clear_current();
  /// clears all data from an approximation interface
  virtual void clear_all();
  /// clears saved data (from pop invocations) from an approximation interface
  virtual void clear_saved();

  /// retrieve the Approximations within an ApproximationInterface
  virtual std::vector<Approximation>& approximations();
  /// retrieve the approximation data from a particular Approximation
  /// within an ApproximationInterface
  virtual const Pecos::SurrogateData& approximation_data(size_t index);

  /// retrieve the approximation coefficients from each Approximation
  /// within an ApproximationInterface
  virtual const RealVectorArray& approximation_coefficients();
  /// set the approximation coefficients within each Approximation
  /// within an ApproximationInterface
  virtual void approximation_coefficients(const RealVectorArray& approx_coeffs);
  /// retrieve the approximation variances from each Approximation
  /// within an ApproximationInterface
  virtual const RealVector& approximation_variances(const Variables& vars);

  /// retrieve the analysis drivers specification for application interfaces
  virtual const StringArray& analysis_drivers() const;

  /// return AnalysisCode::fileNameMap when defined for derived Interface class
  virtual const AnalysisCode* analysis_code() const;

  /// return flag indicating usage of the global evaluation cache
  virtual bool evaluation_cache() const;

  //
  //- Heading: Set and Inquire functions
  //

  /// replaces existing letter with a new one
  void assign_rep(Interface* interface_rep, bool ref_count_incr = true);

  /// returns the interface type
  const String& interface_type() const;

  /// returns the interface identifier
  const String& interface_id() const;

  /// returns the value of the (total) evaluation id counter for the interface
  int evaluation_id() const;

  /// set fineGrainEvalCounters to true and initialize counters if needed
  void fine_grained_evaluation_counters(size_t num_fns);
  /// initialize fine grained evaluation counters
  void init_evaluation_counters(size_t num_fns);
  /// set evaluation count reference points for the interface
  void set_evaluation_reference();

  /// print an evaluation summary for the interface
  void print_evaluation_summary(std::ostream& s, bool minimal_header,
				bool relative_count) const;

  /// returns a flag signaling the use of multiprocessor evaluation partitions
  bool multi_proc_eval_flag() const;

  /// returns a flag signaling the use of a dedicated master processor at the
  /// iterator-evaluation scheduling level
  bool iterator_eval_dedicated_master_flag() const;

  /// function to check interfaceRep (does this envelope contain a letter?)
  bool is_null() const;

protected:

  //
  //- Heading: Constructors
  //

  /// constructor initializes the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  Interface(BaseConstructor, const ProblemDescDB& problem_db);

  /// constructor initializes the base class part of letter classes
  /// (NoDBBaseConstructor used for on the fly instantiations without a DB)
  Interface(NoDBBaseConstructor, size_t num_fns, short output_level);

  //
  //- Heading: Convenience functions
  //

  /// Define algebraicACVIndices, algebraicACVIds, and algebraicFnIndices
  void init_algebraic_mappings(const Variables& vars, const Response& response);

  /// define the evaluation requirements for algebraic_mappings()
  /// (algebraic_set) and the core Application/Approximation mapping (core_set)
  /// from the total Interface evaluation requirements (total_set)
  void asv_mapping(const ActiveSet& total_set, ActiveSet& algebraic_set,
		   ActiveSet& core_set);

  /// evaluate the algebraic_response using the AMPL solver library
  /// and the data extracted from the algebraic_mappings file
  void algebraic_mappings(const Variables& vars, const ActiveSet& algebraic_set,
			  Response& algebraic_response);

  /// combine the response from algebraic_mappings() with the response
  /// from derived_map() to create the total response
  void response_mapping(const Response& algebraic_response,
			const Response& core_response,
			Response& total_response);

  //
  //- Heading: Data
  //

  /// the interface type: system, fork, direct, grid, or approximation
  String interfaceType;
  /// the interface specification identifier string from the DAKOTA input file
  String interfaceId;

  /// flag for the presence of algebraic_mappings that define the subset of an
  /// Interface's parameter to response mapping that is explicit and algebraic.
  bool algebraicMappings;
  /// flag for the presence of non-algebraic mappings that define the core of an
  /// Interface's parameter to response mapping (using analysis_drivers for
  /// ApplicationInterface or functionSurfaces for ApproximationInterface).
  bool coreMappings;

  /// identifier for the current evaluation, which may differ from the
  /// evaluation counters in the case of evaluation scheduling; used on
  /// iterator master as well as server processors.  Currently, this is
  /// set prior to all invocations of derived_map() for all processors.
  int currEvalId;
  // TO DO: what's the right behavior for derived_map_asynch()?  This is
  // secondary, since DirectApplicInterface provides the main use case.

  // evaluation counters specific to each interface instance that track
  // counts on the iterator master processor

  bool fineGrainEvalCounters; ///< controls use of fn val/grad/hess counters
  int evalIdCntr;     ///< total interface evaluation counter
  int newEvalIdCntr;  ///< new (non-duplicate) interface evaluation counter
  int evalIdRefPt;    ///< iteration reference point for evalIdCntr
  int newEvalIdRefPt; ///< iteration reference point for newEvalIdCntr
  // counter arrays provide more detailed reporting if output level >= verbose
  IntArray fnValCounter;     ///< number of value evaluations by resp fn
  IntArray fnGradCounter;    ///< number of gradient evaluations by resp fn
  IntArray fnHessCounter;    ///< number of Hessian evaluations by resp fn
  IntArray newFnValCounter;  ///< number of new value evaluations by resp fn
  IntArray newFnGradCounter; ///< number of new gradient evaluations by resp fn
  IntArray newFnHessCounter; ///< number of new Hessian evaluations by resp fn
  IntArray fnValRefPt;       ///< iteration reference point for fnValCounter
  IntArray fnGradRefPt;      ///< iteration reference point for fnGradCounter
  IntArray fnHessRefPt;      ///< iteration reference point for fnHessCounter
  IntArray newFnValRefPt;    ///< iteration reference point for newFnValCounter
  IntArray newFnGradRefPt;   ///< iteration reference point for newFnGradCounter
  IntArray newFnHessRefPt;   ///< iteration reference point for newFnHessCounter

  /// Set of responses returned after either a blocking or nonblocking schedule
  /// of asynchronous evaluations.
  /** The map is a full/partial set of completions which are identified through
      their evalIdCntr key.  The raw set is postprocessed (i.e., finite diff
      grads merged) in Model::synchronize() where it becomes responseMap. */
  IntResponseMap rawResponseMap;

  /// response function descriptors from the DAKOTA input file (used in
  /// print_evaluation_summary() and derived direct interface classes)
  StringArray fnLabels;

  /// flag for multiprocessor evaluation partitions (evalComm)
  bool multiProcEvalFlag;

  /// flag for dedicated master partitioning at the iterator level
  bool ieDedMasterFlag;

  /// output verbosity level: {SILENT,QUIET,NORMAL,VERBOSE,DEBUG}_OUTPUT
  short outputLevel;

private:

  //
  //- Heading: Member functions
  //

  /// Used by the envelope to instantiate the correct letter class
  Interface* get_interface(ProblemDescDB& problem_db);

  /// Used by algebraic mappings to determine the correct AMPL function
  /// evaluation call to make
  int algebraic_function_type(String);

  //
  //- Heading: Data
  //

  /// set of variable tags from AMPL stub.col
  StringArray algebraicVarTags;
  /// set of indices mapping AMPL algebraic variables to DAKOTA all
  /// continuous variables
  SizetArray algebraicACVIndices;
  /// set of ids mapping AMPL algebraic variables to DAKOTA all
  /// continuous variables
  SizetArray algebraicACVIds;
  /// set of function tags from AMPL stub.row
  StringArray algebraicFnTags;
  /// function type: > 0 = objective, < 0 = constraint
  /// |value|-1 is the objective (constraint) index when making 
  /// AMPL objval (conival) calls
  IntArray algebraicFnTypes;
  /// set of indices mapping AMPL algebraic objective functions to
  /// DAKOTA response functions
  SizetArray algebraicFnIndices;
  /// set of weights for computing Hessian matrices for algebraic constraints;
  RealArray algebraicConstraintWeights;
  /// number of algebraic responses (objectives+constraints)
  int numAlgebraicResponses;

  /// pointer to the letter (initialized only for the envelope)
  Interface* interfaceRep;
  /// number of objects sharing interfaceRep
  int referenceCount;

#ifdef HAVE_AMPL
  /// pointer to an AMPL solver library (ASL) object
  ASL *asl;
#endif // HAVE_AMPL
};


// nonvirtual functions can access letter attributes directly (only need to fwd
// member function call when the function could be redefined).
inline const String& Interface::interface_type() const
{ return (interfaceRep) ? interfaceRep->interfaceType : interfaceType; }


inline const String& Interface::interface_id() const
{ return (interfaceRep) ? interfaceRep->interfaceId : interfaceId; }


inline int Interface::evaluation_id() const
{ return (interfaceRep) ? interfaceRep->evalIdCntr : evalIdCntr; }


inline bool Interface::multi_proc_eval_flag() const
{ return (interfaceRep) ? interfaceRep->multiProcEvalFlag : multiProcEvalFlag; }


inline bool Interface::iterator_eval_dedicated_master_flag() const
{ return (interfaceRep) ? interfaceRep->ieDedMasterFlag : ieDedMasterFlag; }


inline bool Interface::is_null() const
{ return (interfaceRep) ? false : true; }


/// global comparison function for Interface
inline bool interface_id_compare(const Interface& interface, const void* id)
{ return ( *(const String*)id == interface.interface_id() ); }

} // namespace Dakota

#endif
