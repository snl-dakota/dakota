/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DirectApplicInterface
//- Description:  Derived class for the case when analysis code simulators are
//-               linked into the code and may be invoked directly
//- Owner:        Mike Eldred
//- Version: $Id: DirectApplicInterface.H 7024 2010-10-16 01:24:42Z mseldre $

#ifndef DIRECT_APPLIC_INTERFACE_H
#define DIRECT_APPLIC_INTERFACE_H

#include "ApplicationInterface.H"
//#include "DakotaVariables.H"
//#include "DakotaActiveSet.H"
//#include "DakotaResponse.H"
//#ifndef OSF
//#include <pthread.h>
//#endif

#ifdef DAKOTA_MATLAB
#include "engine.h"
//struct engine;  //forward declaration for engine pointer
//struct mxArray; //forward declaration for mxArray argument
#endif
#ifdef DAKOTA_PYTHON
#include <Python.h>  //eventually use forward declaration of PyObject
#endif

namespace Dakota {

/// enumeration of possible variable types (to index to names)
enum var_t { VAR_x1, VAR_x2, VAR_x3, // generic (Rosenbrock, Ishigami)
	     VAR_b, VAR_h, VAR_P, VAR_M, VAR_Y, // short column
	     VAR_w, VAR_t, VAR_R, VAR_E, VAR_X, /* VAR_Y, */ // cantilever beam
	     VAR_Fs, VAR_P1, VAR_P2, VAR_P3, VAR_B, VAR_D, VAR_H,
	     VAR_F0, VAR_d }; /* VAR_b, VAR_h, VAR_E */ // steel column
//enum x3_var_t  { X1, X2, X3 }; // generic up to 3 dimensions
//enum shc_var_t { SHC_B, SHC_H, SHC_P, SHC_M, SHC_Y }; // short column
//enum cb_var_t  { CB_W, CB_T, CB_R, CB_E, CB_X, CB_Y }; // cantilever beam
//enum stc_var_t { STC_FS, STC_P1, STC_P2, STC_P3, STC_B, STC_D, STC_H,
//		   STC_F0, STC_E }; // steel column

/// enumeration of possible driver types (to index to names)
enum driver_t { NO_DRIVER=0, CANTILEVER_BEAM, MOD_CANTILEVER_BEAM,
		CYLINDER_HEAD, EXTENDED_ROSENBROCK, GENERALIZED_ROSENBROCK,
		LF_ROSENBROCK, ROSENBROCK, GERSTNER, SCALABLE_GERSTNER,
		LOGNORMAL_RATIO, MULTIMODAL,
		PLUGIN_ROSENBROCK, PLUGIN_TEXT_BOOK,
		SHORT_COLUMN, LF_SHORT_COLUMN,
		SOBOL_RATIONAL, SOBOL_G_FUNCTION, SOBOL_ISHIGAMI,
		STEEL_COLUMN_COST, STEEL_COLUMN_PERFORMANCE, TEXT_BOOK,
		TEXT_BOOK1, TEXT_BOOK2, TEXT_BOOK3, TEXT_BOOK_OUU,
		SCALABLE_TEXT_BOOK, HERBIE, SMOOTH_HERBIE, SHUBERT,
		SALINAS, MODELCENTER, MATLAB, PYTHON };

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

  void derived_synch(PRPQueue& prp_queue);
  void derived_synch_nowait(PRPQueue& prp_queue);

  int  derived_synchronous_local_analysis(const int& analysis_id);

  const StringArray& analysis_drivers() const;

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
  /// variable and response attributes
  void set_local_data(const Variables& vars, const ActiveSet& set,
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
  StringMultiArray xCLabels;  ///< continuous variable labels
  StringMultiArray xDILabels; ///< discrete integer variable labels
  StringMultiArray xDRLabels; ///< discrete real variable labels

  std::map<String, var_t>    varTypeMap;    ///< map from variable label to enum
  std::map<String, driver_t> driverTypeMap; ///< map from driver name to enum
  std::map<var_t, Real> xCM;  ///< map from var_t enum to continuous value
  std::map<var_t, int>  xDIM; ///< map from var_t enum to discrete int value
  std::map<var_t, Real> xDRM; ///< map from var_t enum to discrete real value

  /// var_t enumerations corresponding to DVV components
  std::vector<var_t> varTypeDVV;
  /// var_t enumerations corresponding to continuous variable labels
  std::vector<var_t> xCMLabels;
  /// var_t enumerations corresponding to discrete integer variable labels
  std::vector<var_t> xDIMLabels;
  /// var_t enumerations corresponding to discrete real variable labels
  std::vector<var_t> xDRMLabels;

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

#ifdef DAKOTA_MATLAB
  /// pointer to the MATLAB engine used for direct evaluations
  engine *matlabEngine;
#endif

private:

  //
  //- Heading: Simulators and test functions
  //

  int cantilever();   ///< scaled cantilever test function for optimization
  int mod_cantilever(); ///< unscaled cantilever test function for UQ
  int cyl_head();     ///< the cylinder head constrained optimization test fn
  int multimodal();   ///< multimodal UQ test function
  int log_ratio();    ///< the log_ratio UQ test function
  int short_column(); ///< the short_column UQ/OUU test function
  int lf_short_column(); ///< a low fidelity short_column test function

  int rosenbrock();  ///< the Rosenbrock optimization and least squares test fn
  int generalized_rosenbrock(); ///< n-dimensional Rosenbrock (Schittkowski)
  int extended_rosenbrock();    ///< n-dimensional Rosenbrock (Nocedal/Wright)
  int lf_rosenbrock(); ///< a low fidelity version of the Rosenbrock function

  int gerstner(); ///< the isotropic/anisotropic Gerstner test function family
  int scalable_gerstner(); ///< scalable versions of the Gerstner test family

  int steel_column_cost(); ///< the steel_column_cost UQ/OUU test function
  int steel_column_perf(); ///< the steel_column_perf UQ/OUU test function

  int sobol_rational();   ///< Sobol SA rational test function
  int sobol_g_function(); ///< Sobol SA discontinuous test function
  int sobol_ishigami();   ///< Sobol SA transcendental test function

  int text_book();          ///< the text_book constrained optimization test function
  int text_book1();         ///< portion of text_book() evaluating the objective fn
  int text_book2();         ///< portion of text_book() evaluating constraint 1
  int text_book3();         ///< portion of text_book() evaluating constraint 2
  int text_book_ouu();      ///< the text_book_ouu OUU test function
  int scalable_text_book(); ///< scalable version of the text_book test function

  // suite of separable test functions TODO add docs
  void herbie1D(size_t der_mode, Real xc_loc, std::vector<Real> &w_and_ders); ///< 1D components of herbie function
  void smooth_herbie1D(size_t der_mode, Real xc_loc, std::vector<Real> &w_and_ders); ///< 1D components of smooth_herbie function
  void shubert1D(size_t der_mode, Real xc_loc, std::vector<Real> &w_and_ders); ///< 1D components of shubert function
  int herbie();        ///< returns the N-D herbie function
  int smooth_herbie(); ///< returns the N-D smooth herbie function
  int shubert();       ///< returns the N-D shubert function
  void separable_combine(Real mult_scale_factor, std::vector<Real> & w, std::vector<Real> & d1w, std::vector<Real> & d2w);  ///< utility to combine components of seperable fns
#ifdef DAKOTA_SALINAS
  int salinas(); ///< direct interface to the SALINAS structural dynamics code
#endif // DAKOTA_SALINAS

#ifdef DAKOTA_MODELCENTER
  int mc_api_run(); ///< direct interface to ModelCenter via API, HKIM 4/3/03
#endif // DAKOTA_MODELCENTER

#ifdef DAKOTA_MATLAB
  /// direct interface to Matlab via API, BMA 11/28/05
  int matlab_engine_run();
  /// check that the dakota_matlab strucutre has the specified field_name and
  /// add if necessary; free structure memory in preparation for new alloc
  int matlab_field_prep(mxArray* dakota_matlab, const char* field_name);
#endif // DAKOTA_MATLAB

#ifdef DAKOTA_PYTHON
  /// direct interface to Python via API, BMA 07/02/07
  int python_run();
  /// whether the user requested numpy data structures
  bool userNumpyFlag;
  /// convert arrays of integer types to Python list or numpy array
  template<class ArrayT, class Size>
  bool python_convert_int(const ArrayT& src, Size size, PyObject** dst);
  /// convert RealVector to Python list or numpy array
  bool python_convert(const RealVector& src, PyObject** dst);
  /// convert RealVector + IntVector + RealVector to Python mixed list 
  /// or numpy double array
  bool python_convert(const RealVector& c_src, const IntVector& di_src,
		      const RealVector& dr_src, PyObject** dst);
  /// convert labels
  bool python_convert(const StringMultiArray& src, PyObject** dst);
  /// convert all labels to single list
  bool python_convert(const StringMultiArray& c_src, 
		      const StringMultiArray& di_src, 
		      const StringMultiArray& dr_src, PyObject** dst);
  /// convert python [list of int or float] or [numpy array of double] to 
  /// RealVector (for fns)
  bool python_convert(PyObject *pyv, RealVector& rv, const int& dim);
  /// convert python [list of int or float] or [numpy array of double] to 
  /// double[], for use as helper in converting gradients
  bool python_convert(PyObject *pyv, double *rv, const int& dim);
  /// convert python [list of lists of int or float] or [numpy array of dbl]
  /// to RealMatrix (for gradients)
  bool python_convert(PyObject *pym, RealMatrix &rm);
  /// convert python [list of lists of int or float] or [numpy array of dbl]
  /// to RealMatrix (used as helper in Hessian conversion)
  bool python_convert(PyObject *pym, RealSymMatrix &rm);
  /// convert python [list of lists of lists of int or float] or 
  /// [numpy array of double] to RealSymMatrixArray (for Hessians)
  bool python_convert(PyObject *pyma, RealSymMatrixArray &rma);
#endif // DAKOTA_PYTHON
};


/** This code provides the derived function used by 
    ApplicationInterface::serve_analyses_synch(). */
inline int DirectApplicInterface::
derived_synchronous_local_analysis(const int& analysis_id)
{ 
  analysisDriverIndex = analysis_id-1;
  return derived_map_ac(analysisDrivers[analysisDriverIndex]);
}


inline const StringArray& DirectApplicInterface::analysis_drivers() const
{ return analysisDrivers; }


//inline void DirectApplicInterface::clear_bookkeeping()
//{ threadIdMap.clear(); }

} // namespace Dakota

#endif
