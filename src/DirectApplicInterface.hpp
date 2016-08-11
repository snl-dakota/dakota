/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DirectApplicInterface
//- Description:  Derived interface class managing cases where analysis code
//-               simulators are linked into the code and may be invoked
//-               directly. This manages parts common to all direct simulators.
//- Owner:        Mike Eldred, Brian Adams
//- Version: $Id: DirectApplicInterface.hpp 7024 2010-10-16 01:24:42Z mseldre $

#ifndef DIRECT_APPLIC_INTERFACE_H
#define DIRECT_APPLIC_INTERFACE_H

#include "ApplicationInterface.hpp"
//#ifndef OSF
//#include <pthread.h>
//#endif

namespace Dakota {

// BMA: Need another design for managing these enums as they (mostly)
// belong in TestDriverInterface, but are primarily used in this
// class. Consider the vision for using them in plug-in or service
// interfaces like Matlab

/// enumeration of possible variable types (to index to names)
enum var_t { VAR_x1, VAR_x2, VAR_x3, // generic (Rosenbrock, Ishigami)
	     VAR_b, VAR_h, VAR_P, VAR_M, VAR_Y, // short column
	     VAR_w, VAR_t, VAR_R, VAR_E, VAR_X, /* VAR_Y, */ // cantilever beam
	     VAR_Fs, VAR_P1, VAR_P2, VAR_P3, VAR_B, VAR_D, VAR_H,
	     VAR_F0, VAR_d, /* VAR_b, VAR_h, VAR_E */ // steel column
	     VAR_MForm }; // mf_*() test functions
//enum x3_var_t  { X1, X2, X3 }; // generic up to 3 dimensions
//enum shc_var_t { SHC_B, SHC_H, SHC_P, SHC_M, SHC_Y }; // short column
//enum cb_var_t  { CB_W, CB_T, CB_R, CB_E, CB_X, CB_Y }; // cantilever beam
//enum stc_var_t { STC_FS, STC_P1, STC_P2, STC_P3, STC_B, STC_D, STC_H,
//		   STC_F0, STC_E }; // steel column

/// enumeration of possible direct driver types (to index to names)
enum driver_t { NO_DRIVER=0, CANTILEVER_BEAM, MOD_CANTILEVER_BEAM,
		CYLINDER_HEAD, EXTENDED_ROSENBROCK, GENERALIZED_ROSENBROCK,
		LF_ROSENBROCK, EXTRA_LF_ROSENBROCK, MF_ROSENBROCK, MODIFIED_ROSENBROCK,
		ROSENBROCK, LF_POLY_PROD, POLY_PROD,
		GERSTNER, SCALABLE_GERSTNER, LOGNORMAL_RATIO, MULTIMODAL,
		PLUGIN_ROSENBROCK, PLUGIN_TEXT_BOOK,
		SHORT_COLUMN, LF_SHORT_COLUMN, MF_SHORT_COLUMN,
		SIDE_IMPACT_COST, SIDE_IMPACT_PERFORMANCE,
		SOBOL_RATIONAL, SOBOL_G_FUNCTION, SOBOL_ISHIGAMI,
		STEEL_COLUMN_COST, STEEL_COLUMN_PERFORMANCE, TEXT_BOOK,
		TEXT_BOOK1, TEXT_BOOK2, TEXT_BOOK3, TEXT_BOOK_OUU,
		SCALABLE_TEXT_BOOK, SCALABLE_MONOMIALS,
		MOGATEST1, MOGATEST2, MOGATEST3,
		ILLUMINATION, BARNES, BARNES_LF,
		HERBIE, SMOOTH_HERBIE, SHUBERT,
		SALINAS, MODELCENTER, GENZ, DAMPED_OSCILLATOR,
		ANISOTROPIC_QUADRATIC_FORM , BAYES_LINEAR,
		STEADY_STATE_DIFFUSION_1D, TRANSIENT_DIFFUSION_1D,
		PREDATOR_PREY};

/// enumeration for how local variables are stored (values must employ
/// a bit representation)
enum local_data_t { VARIABLES_MAP=1, VARIABLES_VECTOR=2 };


/// Derived application interface class which spawns simulation codes
/// and testers using direct procedure calls.

/** DirectApplicInterface uses a few linkable simulation codes and several
    internal member functions to perform parameter to response mappings. */
class DirectApplicInterface: public ApplicationInterface
{
public:

  //
  //- Heading: Constructor and destructor
  //

  DirectApplicInterface(const ProblemDescDB& problem_db); ///< constructor
  ~DirectApplicInterface();                               ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  void derived_map(const Variables& vars, const ActiveSet& set,
		   Response& response, int fn_eval_id);
  void derived_map_asynch(const ParamResponsePair& pair);

  void wait_local_evaluations(PRPQueue& prp_queue);
  void test_local_evaluations(PRPQueue& prp_queue);

  int  synchronous_local_analysis(int analysis_id);

  const StringArray& analysis_drivers() const;

  void init_communicators_checks(int max_eval_concurrency);
  void  set_communicators_checks(int max_eval_concurrency);

  //void clear_bookkeeping(); // clears threadIdMap

protected:

  //
  //- Heading: New virtual fns (redefined by derived interface plug-ins)
  //

  /// execute the input filter portion of a direct evaluation invocation
  virtual int derived_map_if(const Dakota::String& if_name);
  /// execute an analysis code portion of a direct evaluation invocation
  virtual int derived_map_ac(const Dakota::String& ac_name);
  /// execute the output filter portion of a direct evaluation invocation
  virtual int derived_map_of(const Dakota::String& of_name);

  //
  //- Heading: Methods
  //

  /// convenience function for local test simulators which sets per-evaluation
  /// variable and active set attributes; derived classes
  /// reimplementing this likely need to invoke the base class API
  virtual void set_local_data(const Variables& vars, const ActiveSet& set);
  /// convenience function for local test simulators which sets per-evaluation
  /// response attributes; derived classes
  /// reimplementing this likely need to invoke the base class API
  virtual void set_local_data(const Response& response);
  /// convenience function for local test simulators which sets per-evaluation
  /// variable, active set, and response attributes; derived classes
  /// reimplementing this likely need to invoke the base class API
  virtual void set_local_data(const Variables& vars, const ActiveSet& set,
		      const Response& response);

  /// convenience function for local test simulators which overlays
  /// response contributions from multiple analyses using MPI_Reduce
  void overlay_response(Response& response);

  //
  //- Heading: Data
  //

  String iFilterName; ///< name of the direct function input filter
  String oFilterName; ///< name of the direct function output filter
  driver_t iFilterType; ///< enum type of the direct function input filter
  driver_t oFilterType; ///< enum type of the direct function output filter

  // map of pthread id's to function evaluation id's for asynch evaluations
  //std::map<pthread_t, int> threadIdMap;

  // data used by direct fns is class scope to allow common utility usage
  bool gradFlag;  ///< signals use of fnGrads in direct simulator functions
  bool hessFlag;  ///< signals use of fnHessians in direct simulator functions

  size_t numFns;  ///< number of functions in fnVals
  size_t numVars; ///< total number of continuous and discrete variables
  size_t numACV;  ///< total number of continuous variables
  size_t numADIV;  ///< total number of discete integer variables
  size_t numADRV;  ///< total number of discete real variables
  size_t numADSV;  ///< total number of discete string variables
  size_t numDerivVars; ///< number of active derivative variables

  /// bit-wise record of which local data views are active;
  /// see enum local_data_t
  unsigned short localDataView;

  // Keeping a copy of the data passed to derived_map allows common prototypes
  // with SysCall and ForkApplicInterface in virtual functions (since SysCall
  // and Fork don't need vars/asv/response outside of write_parameters/
  // read_responses.
  //Variables directFnVars; ///< class scope variables object
  RealVector xC;  ///< continuous variables used within direct simulator fns
  IntVector  xDI; ///< discrete int variables used within direct simulator fns
  RealVector xDR; ///< discrete real variables used within direct simulator fns
  StringMultiArray xDS; ///< discrete string variables used within direct simulator fns
  StringMultiArray xCLabels;  ///< continuous variable labels
  StringMultiArray xDILabels; ///< discrete integer variable labels
  StringMultiArray xDRLabels; ///< discrete real variable labels
  StringMultiArray xDSLabels; ///< discrete string variable labels

  std::map<String, var_t>    varTypeMap;    ///< map from variable label to enum
  std::map<String, driver_t> driverTypeMap; ///< map from driver name to enum
  std::map<var_t, Real> xCM;  ///< map from var_t enum to continuous value
  std::map<var_t, int>  xDIM; ///< map from var_t enum to discrete int value
  std::map<var_t, Real> xDRM; ///< map from var_t enum to discrete real value
  std::map<var_t, String> xDSM; ///< map from var_t enum to discrete string value

  /// var_t enumerations corresponding to DVV components
  std::vector<var_t> varTypeDVV;
  /// var_t enumerations corresponding to continuous variable labels
  std::vector<var_t> xCMLabels;
  /// var_t enumerations corresponding to discrete integer variable labels
  std::vector<var_t> xDIMLabels;
  /// var_t enumerations corresponding to discrete real variable labels
  std::vector<var_t> xDRMLabels;
  /// var_t enumerations corresponding to discrete string variable labels
  std::vector<var_t> xDSMLabels;

  //ActiveSet directFnActSet; // class scope ActiveSet object
  ShortArray directFnASV; ///< class scope active set vector
  SizetArray directFnDVV; ///< class scope derivative variables vector

  //Response  directFnResponse; // class scope response object
  RealVector      fnVals;    ///< response fn values within direct simulator fns
  RealMatrix      fnGrads;   ///< response fn gradients w/i direct simulator fns
  RealSymMatrixArray fnHessians; ///< response fn Hessians within direct fns

  /// the set of analyses within each function evaluation (from the
  /// analysis_drivers interface specification)
  StringArray analysisDrivers;
  /// conversion of analysisDrivers to driver_t
  std::vector<driver_t> analysisDriverTypes;

  /// the index of the active analysis driver within analysisDrivers
  size_t analysisDriverIndex;
  /// the set of optional analysis components used by the analysis drivers
  /// (from the analysis_components interface specification)
  String2DArray analysisComponents;

private:
  /// map labels in src to var_t in dest
  void map_labels_to_enum(StringMultiArrayConstView &src,
      std::vector<var_t> &dest);
};


/** This code provides the derived function used by
    ApplicationInterface::serve_analyses_synch(). */
inline int DirectApplicInterface::synchronous_local_analysis(int analysis_id)
{
  analysisDriverIndex = analysis_id-1;
  return derived_map_ac(analysisDrivers[analysisDriverIndex]);
}


inline const StringArray& DirectApplicInterface::analysis_drivers() const
{ return analysisDrivers; }


/** Process init issues as warnings since some contexts (e.g.,
    HierarchSurrModel) initialize more configurations than will be
    used and DirectApplicInterface allows override by derived plug-ins. */
inline void DirectApplicInterface::
init_communicators_checks(int max_eval_concurrency)
{
  bool warn = true;
  check_asynchronous(warn, max_eval_concurrency);
  check_multiprocessor_asynchronous(warn, max_eval_concurrency);
}


/** Process run-time issues as hard errors. */
inline void DirectApplicInterface::
set_communicators_checks(int max_eval_concurrency)
{
  bool warn = false,  mp1 = check_asynchronous(warn, max_eval_concurrency),
       mp2 = check_multiprocessor_asynchronous(warn, max_eval_concurrency);
  if (mp1 || mp2)
    abort_handler(-1);
}


//inline void DirectApplicInterface::clear_bookkeeping()
//{ threadIdMap.clear(); }


inline void DirectApplicInterface::
set_local_data(const Variables& vars, const ActiveSet& set,
	       const Response& response)
{ set_local_data(vars, set); set_local_data(response); }

} // namespace Dakota

#endif
