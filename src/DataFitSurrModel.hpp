/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       DataFitSurrModel
//- Description: A model which manages a surrogate relationship between a
//-              data fit approximation (local, multipoint, or global) and
//-              a truth model.
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: DataFitSurrModel.hpp 7029 2010-10-22 00:17:02Z mseldre $

#ifndef DATA_FIT_SURR_MODEL_H
#define DATA_FIT_SURR_MODEL_H

#include "dakota_data_types.hpp"
#include "SurrogateModel.hpp"
#include "DakotaInterface.hpp"
#include "DakotaIterator.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"


namespace Dakota {

/// Derived model class within the surrogate model branch for managing
/// data fit surrogates (global and local)

/** The DataFitSurrModel class manages global or local approximations
    (surrogates that involve data fits) that are used in place of an
    expensive model.  The class contains an approxInterface (required
    for both global and local) which manages the approximate function
    evaluations, an actualModel (optional for global, required for
    local) which provides truth evaluations for building the
    surrogate, and a daceIterator (optional for global, not used for
    local) which selects parameter sets on which to evaluate
    actualModel in order to generate the necessary data for building
    global approximations. */

class DataFitSurrModel: public SurrogateModel
{
public:
  
  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  DataFitSurrModel(ProblemDescDB& problem_db);
  /// alternate constructor for instantiations on the fly
  DataFitSurrModel(Iterator& dace_iterator, Model& actual_model,
		   //const std::pair<short,short>& view,
		   //const Sizet2DArray& vars_comps, const ActiveSet& set,
		   const String& approx_type, const UShortArray& approx_order,
		   short corr_type, short corr_order, short data_order,
		   short output_level, const String& point_reuse,
		   const String& export_points_file = String(),
		   bool export_annotated = true,
		   const String& import_points_file = String(),
		   bool import_annotated = true);
  /// destructor
  ~DataFitSurrModel();

  //
  //- Heading: Member functions
  //

  /// set pointsTotal and pointsManagement mode
  void total_points(int points);

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  // Perform the response computation portions specific to this derived 
  // class.  In this case, it simply employs approxInterface.map()/synch()/
  // synch_nowait() where approxInterface is a local, multipoint, or global
  // approximation.
  //
  /// portion of compute_response() specific to DataFitSurrModel
  void derived_compute_response(const ActiveSet& set);
  /// portion of asynch_compute_response() specific to DataFitSurrModel
  void derived_asynch_compute_response(const ActiveSet& set);
  /// portion of synchronize() specific to DataFitSurrModel
  const IntResponseMap& derived_synchronize();
  /// portion of synchronize_nowait() specific to DataFitSurrModel
  const IntResponseMap& derived_synchronize_nowait();

  /// return daceIterator
  Iterator& subordinate_iterator();
  /// return this model instance
  Model& surrogate_model();
  /// return actualModel
  Model& truth_model();
  /// return actualModel (and optionally its sub-models)
  void derived_subordinate_models(ModelList& ml, bool recurse_flag);
  /// pass request to actualModel if recursing and then update from it
  void update_from_subordinate_model(bool recurse_flag = true);
  /// return approxInterface
  Interface& interface();

  /// set the relative weightings for multiple objective functions or least
  /// squares terms and optionally recurses into actualModel
  void primary_response_fn_weights(const RealVector& wts,
				   bool recurse_flag = true);

  /// set responseMode and pass any bypass request on to actualModel for
  /// any lower-level surrogates.
  void surrogate_response_mode(short mode);

  /// (re)set the surrogate index set in SurrogateModel::surrogateFnIndices
  /// and ApproximationInterface::approxFnIndices
  void surrogate_function_indices(const IntSet& surr_fn_indices);

  /// Builds the local/multipoint/global approximation using
  /// daceIterator/actualModel to generate new data points
  void build_approximation();
  /// Builds the local/multipoint/global approximation using
  /// daceIterator/actualModel to generate new data points that
  /// augment the vars/response anchor point
  bool build_approximation(const Variables& vars,
			   const IntResponsePair& response_pr);

  /// replaces the approximation data with daceIterator results and
  /// rebuilds the approximation if requested
  void update_approximation(bool rebuild_flag);
  /// replaces the anchor point, and rebuilds the approximation if requested
  void update_approximation(const Variables& vars,
			    const IntResponsePair& response_pr,
			    bool rebuild_flag);
  /// replaces the current points array and rebuilds the approximation
  /// if requested
  void update_approximation(const VariablesArray& vars_array,
			    const IntResponseMap& resp_map, bool rebuild_flag);

  /// appends daceIterator results to a global approximation and rebuilds
  /// it if requested
  void append_approximation(bool rebuild_flag);
  /// appends a point to a global approximation and rebuilds it if requested
  void append_approximation(const Variables& vars,
			    const IntResponsePair& response_pr,
			    bool rebuild_flag);
  /// appends an array of points to a global approximation and rebuilds it
  /// if requested
  void append_approximation(const VariablesArray& vars_array,
			    const IntResponseMap& resp_map, bool rebuild_flag);

  /// remove approximation data added on previous append_approximation() call
  /// or a specified number of points
  void pop_approximation(bool save_surr_data, bool rebuild_flag = false);

  /// restore a previous approximation data state
  void restore_approximation();
  /// query for whether a trial increment is restorable
  bool restore_available();

  /// finalize data fit by applying all previous trial increments
  void finalize_approximation();

  /// store the current data fit approximation for later combination
  void store_approximation();
  /// combine the current data fit approximation with one previously stored
  void combine_approximation(short corr_type);

  /// retrieve the set of Approximations from approxInterface
  std::vector<Approximation>& approximations();
  /// return the approximation coefficients from each Approximation
  /// (request forwarded to approxInterface)
  const RealVectorArray& approximation_coefficients();
  /// set the approximation coefficients within each Approximation
  /// (request forwarded to approxInterface)
  void approximation_coefficients(const RealVectorArray& approx_coeffs);
  /// return the approximation variance from each Approximation
  /// (request forwarded to approxInterface)
  const RealVector& approximation_variances(const Variables& vars);
  /// return the approximation data from a particular Approximation
  /// (request forwarded to approxInterface)
  const Pecos::SurrogateData& approximation_data(size_t index);

  /// update component parallel mode for supporting parallelism in actualModel
  void component_parallel_mode(short mode);

  /// set up actualModel for parallel operations
  void derived_init_communicators(int max_iterator_concurrency,
				  bool recurse_flag = true);
  /// set up actualModel for serial operations.
  void derived_init_serial();
  /// set active parallel configuration within actualModel
  void derived_set_communicators(int max_iterator_concurrency,
				 bool recurse_flag = true);
  /// deallocate communicator partitions for the DataFitSurrModel
  /// (request forwarded to actualModel)
  void derived_free_communicators(int max_iterator_concurrency,
				  bool recurse_flag = true);

  /// Service actualModel job requests received from the master.
  /// Completes when a termination message is received from stop_servers().
  void serve();
  /// Executed by the master to terminate actualModel server operations
  /// when DataFitSurrModel iteration is complete.
  void stop_servers();

  /// update the Model's inactive view based on higher level (nested)
  /// context and optionally recurse into actualModel
  void inactive_view(short view, bool recurse_flag = true);

  /// return the approxInterface identifier
  const String& interface_id() const;
  /// return the current evaluation id for the DataFitSurrModel
  int evaluation_id() const;

  /// set the evaluation counter reference points for the DataFitSurrModel
  /// (request forwarded to approxInterface and actualModel)
  void set_evaluation_reference();
  /// request fine-grained evaluation reporting within approxInterface
  /// and actualModel
  void fine_grained_evaluation_counters();
  /// print the evaluation summary for the DataFitSurrModel
  /// (request forwarded to approxInterface and actualModel)
  void print_evaluation_summary(std::ostream& s, bool minimal_header = false,
				bool relative_count = true) const;

private:

  //
  //- Heading: Convenience functions
  //

  /// optionally read surrogate data points from provided file
  void import_points(bool annotated);

  /// Common code for processing of approximate response maps shared by
  /// derived_synchronize() and derived_synchronize_nowait()
  void derived_synchronize_approx(const IntResponseMap& approx_resp_map,
				  IntResponseMap& approx_resp_map_rekey);

  /// Updates fit arrays for global approximations
  void update_global();
  /// Updates fit arrays for local or multipoint approximations
  void update_local_multipoint();
  /// Builds a global approximation using daceIterator
  void build_global();
  /// Builds a local or multipoint approximation using actualModel
  void build_local_multipoint();

  /// update actualModel with data from current variables/labels/bounds/targets
  void update_actual_model();
  /// update current variables/labels/bounds/targets with data from actualModel
  void update_from_actual_model();

  /// test if c_vars and d_vars are within [c_l_bnds,c_u_bnds] and
  /// [d_l_bnds,d_u_bnds]
  bool inside(const RealVector& c_vars, const IntVector& di_vars,
	      const RealVector& dr_vars);

  //
  //- Heading: Data members
  //

  /// number of calls to derived_compute_response()/
  /// derived_asynch_compute_response()
  int surrModelEvalCntr;

  /// total points the user specified to construct the surrogate
  int pointsTotal;
  /// configuration for points management in build_global()
  short pointsManagement;

  /// type of point reuse for approximation builds: \c all, \c region
  /// (default if points file), or \c none (default if no points file)
  String pointReuse;
  /// file name from \c import_points_file specification
  String importPointsFile;
  /// file name from \c export_points_file specification
  String exportPointsFile;
  /// annotation setting for file export of variables and approximate responses
  bool exportAnnotated;
  /// file name for \c export_points_file specification
  std::ofstream exportFileStream;
  /// array of variables sets read from the \c import_points_file
  VariablesList reuseFileVars;
  /// array of response sets read from the \c import_points_file
  ResponseList reuseFileResponses;

  /// manages the building and subsequent evaluation of the approximations
  /// (required for both global and local)
  Interface approxInterface;

  /// the truth model which provides evaluations for building the surrogate
  /// (optional for global, required for local)
  /** actualModel is unrestricted in type; arbitrary nestings are possible. */
  Model actualModel;

  /// selects parameter sets on which to evaluate actualModel in order
  /// to generate the necessary data for building global approximations
  /// (optional for global since restart data may also be used)
  Iterator daceIterator;
};


/** Virtual destructor handles referenceCount at Strategy level. */
inline DataFitSurrModel::~DataFitSurrModel()
{
  if (!exportPointsFile.empty())
    exportFileStream.close();
}


inline void DataFitSurrModel::total_points(int points)
{ pointsTotal = points; if (points > 0) pointsManagement = TOTAL_POINTS; }


inline Iterator& DataFitSurrModel::subordinate_iterator()
{ return daceIterator; }


inline Model& DataFitSurrModel::surrogate_model()
{
  // return by reference: OK to return letter instance
  return *this;

  // return by value: letter instance must be returned within an envelope for
  // representation sharing/reference counting to work properly
  //Model surr_model;            // empty envelope
  //surr_model.assign_rep(this); // populate letter, increment reference count
  //return surr_model;
}


inline Model& DataFitSurrModel::truth_model()
{ return actualModel; }


inline void DataFitSurrModel::
derived_subordinate_models(ModelList& ml, bool recurse_flag)
{
  if (!actualModel.is_null()) {
    ml.push_back(actualModel);
    if (recurse_flag)
      actualModel.derived_subordinate_models(ml, recurse_flag);
  }
}


inline void DataFitSurrModel::update_from_subordinate_model(bool recurse_flag)
{
  if (probDescDB.is_null()) { // instantiation on-the-fly
    // data flows from the bottom-up, so recurse first
    if (recurse_flag && !actualModel.is_null())
      actualModel.update_from_subordinate_model(recurse_flag);
    // now pull the latest updates from actualModel
    update_from_actual_model();
  }
}


inline Interface& DataFitSurrModel::interface()
{ return approxInterface; }


inline void DataFitSurrModel::
primary_response_fn_weights(const RealVector& wts, bool recurse_flag)
{
  primaryRespFnWts = wts;
  if (recurse_flag && !actualModel.is_null())
    actualModel.primary_response_fn_weights(wts, recurse_flag);
}


inline void DataFitSurrModel::surrogate_response_mode(short mode)
{
  responseMode = mode;
  if (mode == BYPASS_SURROGATE) // recurse in this case
    actualModel.surrogate_response_mode(mode);
}


inline void DataFitSurrModel::
surrogate_function_indices(const IntSet& surr_fn_indices)
{
  surrogateFnIndices = surr_fn_indices;
  approxInterface.approximation_function_indices(surr_fn_indices);
}


inline bool DataFitSurrModel::restore_available()
{ return approxInterface.restore_available(); }


inline std::vector<Approximation>& DataFitSurrModel::approximations()
{ return approxInterface.approximations(); }


inline const RealVectorArray& DataFitSurrModel::approximation_coefficients()
{ return approxInterface.approximation_coefficients(); }


inline void DataFitSurrModel::
approximation_coefficients(const RealVectorArray& approx_coeffs)
{
  approxInterface.approximation_coefficients(approx_coeffs);

  // Surrogate data is being imported.  Update state to suppress automatic
  // surrogate construction.
  approxBuilds++;
  if (strbegins(surrogateType, "global_"))
    update_global();
  else
    update_local_multipoint();
}


inline const RealVector& DataFitSurrModel::
approximation_variances(const Variables& vars)
{ return approxInterface.approximation_variances(vars); }


inline const Pecos::SurrogateData& DataFitSurrModel::
approximation_data(size_t index)
{ return approxInterface.approximation_data(index); }


/** asynchronous flags need to be initialized for the sub-models.  In addition,
    max_iterator_concurrency is the outer level iterator concurrency, not the
    DACE concurrency that actualModel will see, and recomputing the
    message_lengths on the sub-model is probably not a bad idea either.
    Therefore, recompute everything on actualModel using init_communicators. */
inline void DataFitSurrModel::
derived_init_communicators(int max_iterator_concurrency, bool recurse_flag)
{
  // initialize approxInterface (for serial operations).
  // Note: this is where max_iterator_concurrency would be used.
  //approxInterface.init_serial();

  // initialize actualModel for parallel operations
  if (recurse_flag && !actualModel.is_null()) {
    // minimum_points() returns the minimum number of points needed to build
    // approxInterface (global and local approximations) without any numerical
    // derivatives multiplier.  Obtain the deriv multiplier from actualModel.
    // min_points does not account for reuse_points or anchor, since these
    // will vary, and min_points must remain constant among ctor/run/dtor.
    int min_conc = approxInterface.minimum_points(false)
                 * actualModel.derivative_concurrency();
    if (daceIterator.is_null()) {
      // store within empty envelope for later use in derived_{set,free}_comms
      daceIterator.maximum_concurrency(min_conc);
      actualModel.init_communicators(min_conc);
    }
    else {
      // daceIterator.maximum_concurrency() includes number of user-specified
      // samples for building a global approx & any numerical deriv multiplier.
      // DakotaIterator::maxConcurrency must remain constant for ctor/run/dtor.
      int dace_conc = daceIterator.maximum_concurrency();
      // The concurrency for global/local surrogate construction is defined by
      // the greater of the dace samples user-specification and the min_points
      // approximation requirement.
      if (min_conc > dace_conc) {
	dace_conc = min_conc;
	daceIterator.maximum_concurrency(dace_conc); // update
      }
      actualModel.init_communicators(dace_conc);
    }
  }
}


inline void DataFitSurrModel::derived_init_serial()
{
  //approxInterface.init_serial();

  if (!actualModel.is_null())
    actualModel.init_serial();
}


inline void DataFitSurrModel::
derived_set_communicators(int max_iterator_concurrency, bool recurse_flag)
{
  //parallelLib.parallel_configuration_iterator(modelPCIter);
  //approxInterface.set_communicators(messageLengths);

  if (recurse_flag && !actualModel.is_null())
    actualModel.set_communicators(daceIterator.maximum_concurrency());
}


inline void DataFitSurrModel::
derived_free_communicators(int max_iterator_concurrency, bool recurse_flag)
{
  //parallelLib.parallel_configuration_iterator(modelPCIter);
  //approxInterface.free_communicators();

  if (recurse_flag && !actualModel.is_null())
    actualModel.free_communicators(daceIterator.maximum_concurrency());
}


inline void DataFitSurrModel::serve()
{
  if (!actualModel.is_null())
    actualModel.serve();
}


inline void DataFitSurrModel::stop_servers()
{
  if (!actualModel.is_null())
    actualModel.stop_servers();
}


inline void DataFitSurrModel::inactive_view(short view, bool recurse_flag)
{
  currentVariables.inactive_view(view);
  userDefinedConstraints.inactive_view(view);
  if (recurse_flag && !actualModel.is_null())
    actualModel.inactive_view(view, recurse_flag);
}


inline const String& DataFitSurrModel::interface_id() const
{ return approxInterface.interface_id(); }


/** return the DataFitSurrModel evaluation count.  Due to possibly
    intermittent use of surrogate bypass, this is not the same as
    either the approxInterface or actualModel model evaluation counts.
    It also does not distinguish duplicate evals. */
inline int DataFitSurrModel::evaluation_id() const
{ return surrModelEvalCntr; }


inline void DataFitSurrModel::set_evaluation_reference()
{
  approxInterface.set_evaluation_reference();

  // don't recurse this, since the eval reference is for the top level iteration
  //if (!actualModel.is_null())
  //  actualModel.set_evaluation_reference();

  // may want to add this in time
  //surrModelEvalRef = surrModelEvalCntr;
}


inline void DataFitSurrModel::fine_grained_evaluation_counters()
{
  approxInterface.fine_grained_evaluation_counters(numFns);
  if (!actualModel.is_null())
    actualModel.fine_grained_evaluation_counters();
}


inline void DataFitSurrModel::
print_evaluation_summary(std::ostream& s, bool minimal_header,
			 bool relative_count) const
{
  approxInterface.print_evaluation_summary(s, minimal_header, relative_count);
  if (!actualModel.is_null()) {
    if (daceIterator.is_null())
      actualModel.print_evaluation_summary(s, minimal_header, relative_count);
    else // daceIterator resets the eval reference -> don't use a relative count
      actualModel.print_evaluation_summary(s, minimal_header, false);
  }
}

} // namespace Dakota

#endif
