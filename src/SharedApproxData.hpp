/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SharedApproxData
//- Description:  Abstract base class for shared approxiamtion data
//-               
//- Owner:        Mike Eldred

#ifndef SHARED_APPROX_DATA_H
#define SHARED_APPROX_DATA_H

#include "dakota_data_util.hpp"

namespace Pecos {
class MultivariateDistribution;
class ExpansionConfigOptions;
class BasisConfigOptions;
class RegressionConfigOptions;
}

namespace Dakota {

class ProblemDescDB;


/// Base class for the shared approximation data class hierarchy.

/** The SharedApproxData class is the base class for the shared
    approximation data class hierarchy in DAKOTA.  For memory
    efficiency and enhanced polymorphism, the approximation hierarchy
    employs the "letter/envelope idiom" (see Coplien "Advanced C++",
    p. 133), for which the base class (SharedApproxData) serves as the
    envelope and one of the derived classes (selected in
    SharedApproxData::get_shared_data()) serves as the letter. */

class SharedApproxData
{
  //
  //- Heading: Friends
  //

  friend class Approximation;
  friend class TaylorApproximation;
  friend class TANA3Approximation;
  friend class QMEApproximation;
  friend class GaussProcApproximation;
  friend class VPSApproximation;
  friend class PecosApproximation;
#ifdef HAVE_C3
  friend class C3Approximation;
#endif // HAVE_C3
#ifdef HAVE_SURFPACK
  friend class SurfpackApproximation;
#endif // HAVE_SURFPACK

public:

  //
  //- Heading: Constructors, destructor, assignment operator
  //

  /// default constructor
  SharedApproxData();
  /// standard constructor for envelope
  SharedApproxData(ProblemDescDB& problem_db, size_t num_vars);
   /// alternate constructor for envelope
  SharedApproxData(const String& approx_type, const UShortArray& approx_order,
		   size_t num_vars, short data_order, short output_level);
  /// copy constructor
  SharedApproxData(const SharedApproxData& approx);

  /// destructor
  virtual ~SharedApproxData();

  /// assignment operator
  SharedApproxData operator=(const SharedApproxData& approx);

  //
  //- Heading: Virtual functions
  //

  /// activate an approximation state based on its multi-index key
  virtual void active_model_key(const UShortArray& mi_key);
  /// return active multi-index key
  virtual const UShortArray& active_model_key() const;
  /// reset initial state by clearing all model keys for an approximation
  virtual void clear_model_keys();

  /// define data keys and active data index for aggregated response data
  /// (SharedPecosApproxData)
  virtual void link_multilevel_surrogate_data();

  /// update approxDataKeys[activeDataIndex] with trailing surrogate key
  virtual void surrogate_model_key(const UShortArray& key);
  /// return trailing surrogate key from approxDataKeys[activeDataIndex]
  virtual const UShortArray& surrogate_model_key() const;
  /// update approxDataKeys[activeDataIndex] with leading truth key
  virtual void truth_model_key(const UShortArray& key);
  /// return leading truth key from approxDataKeys[activeDataIndex]
  virtual const UShortArray& truth_model_key() const;

  /// builds the shared approximation data from scratch
  virtual void build();

  /// rebuilds the shared approximation data incrementally
  virtual void rebuild();
  /// back out the previous increment to the shared approximation data 
  virtual void pop(bool save_surr_data);
  /// queries availability of pushing data associated with a trial set
  virtual bool push_available();
  /// return index for restoring trial set within stored data sets
  virtual size_t push_index(const UShortArray& key);
  /// push a previous state of the shared approximation data 
  virtual void pre_push();
  /// clean up popped bookkeeping following push 
  virtual void post_push();
  /// return index of i-th trial set within restorable bookkeeping sets
  virtual size_t finalize_index(size_t i, const UShortArray& key);
  /// finalize the shared approximation data following a set of increments
  virtual void pre_finalize();
  /// clean up popped bookkeeping following aggregation
  virtual void post_finalize();

  /// clear inactive approximation data
  virtual void clear_inactive();

  /// aggregate the shared approximation data from current and stored states
  virtual void pre_combine();
  /// clean up stored data sets after aggregation
  virtual void post_combine();
  /// promote aggregated data sets to active state
  virtual void combined_to_active(bool clear_combined = true);

  /// propagate updates to random variable distribution parameters to a
  /// polynomial basis
  virtual void update_basis_distribution_parameters(
    const Pecos::MultivariateDistribution& mvd);

  /// set ExpansionConfigOptions instance as a group specification
  virtual void configuration_options(
    const Pecos::ExpansionConfigOptions& ec_options);
  /// set BasisConfigOptions instance as a group specification
  virtual void configuration_options(
    const Pecos::BasisConfigOptions& bc_options);
  /// set BasisConfigOptions instance as a group specification
  virtual void configuration_options(
    const Pecos::RegressionConfigOptions& rc_options);

  /// assign key identifying a subset of variables that are to be
  /// treated as random for statistical purposes (e.g. expectation)
  virtual void random_variables_key(const BitArray& random_vars_key);

  /// assign statistics mode: {ACTIVE,COMBINED}_EXPANSION_STATS
  virtual void refinement_statistics_type(short stats_type);

  /// return set of Sobol indices that have been requested (e.g., as constrained
  /// by throttling) and are computable by a (sparse) expansion of limited order
  virtual const Pecos::BitArrayULongMap& sobol_index_map() const;

  //
  //- Heading: Member functions
  //

  // return the number of variables used in the approximation
  //int num_variables() const;

  /// set activeDataIndex
  void surrogate_data_index(size_t d_index);

  /// set approximation lower and upper bounds (currently only used by graphics)
  void set_bounds(const RealVector&  c_l_bnds, const RealVector&  c_u_bnds,
		  const IntVector&  di_l_bnds, const IntVector&  di_u_bnds,
		  const RealVector& dr_l_bnds, const RealVector& dr_u_bnds);

  /// returns dataRep for access to derived class member functions
  /// that are not mapped to the top SharedApproxData level
  SharedApproxData* data_rep() const;

protected:

  //
  //- Heading: Constructors
  //

  /// constructor initializes the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  SharedApproxData(BaseConstructor, ProblemDescDB& problem_db, size_t num_vars);

  /// constructor initializes the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  SharedApproxData(NoDBBaseConstructor, const String& approx_type,
		   size_t num_vars, short data_order, short output_level);

  //
  //- Heading: Data
  //

  /// number of variables in the approximation
  size_t numVars;

  /// approximation type identifier
  String approxType;

  /// order of the data used for surrogate construction, in ActiveSet
  /// request vector 3-bit format.
  /** This setting distinguishes derivative data intended for use in
      construction (includes derivatives w.r.t. the build variables) from
      derivative data that may be approximated separately (excludes derivatives
      w.r.t. auxilliary variables).  This setting should also not be inferred
      directly from the responses specification, since we may need gradient
      support for evaluating gradients at a single point (e.g., the center of
      a trust region), but not require gradient evaluations at every point. */
  short buildDataOrder;

  /// output verbosity level: {SILENT,QUIET,NORMAL,VERBOSE,DEBUG}_OUTPUT
  short outputLevel;

  /// index of active approxData instance 
  size_t activeDataIndex;
  /// set of multi-index model keys (#surrData x #numKeys) to enumerate
  /// when updating SurrogateData instances within each Approximation
  UShort3DArray approxDataKeys;

  /// Prefix for model export files
  String modelExportPrefix;
  /// Bitmapped format request for exported models
  unsigned short modelExportFormat;

  /// approximation continuous lower bounds (used by 3D graphics and
  /// Surfpack KrigingModel)
  RealVector approxCLowerBnds;
  /// approximation continuous upper bounds (used by 3D graphics and
  /// Surfpack KrigingModel)
  RealVector approxCUpperBnds;
  /// approximation continuous lower bounds
  IntVector approxDILowerBnds;
  /// approximation continuous upper bounds
  IntVector approxDIUpperBnds;
  /// approximation continuous lower bounds
  RealVector approxDRLowerBnds;
  /// approximation continuous upper bounds
  RealVector approxDRUpperBnds;

private:

  //
  //- Heading: Member functions
  //

  /// Used only by the standard envelope constructor to initialize
  /// dataRep to the appropriate derived type.
  SharedApproxData* get_shared_data(ProblemDescDB& problem_db, size_t num_vars);

  /// Used only by the alternate envelope constructor to initialize
  /// dataRep to the appropriate derived type.
  SharedApproxData* get_shared_data(const String& approx_type,
				    const UShortArray& approx_order,
				    size_t num_vars, short data_order,
				    short output_level);

  //
  //- Heading: Data
  //

  /// pointer to the letter (initialized only for the envelope)
  SharedApproxData* dataRep;
  /// number of objects sharing dataRep
  int referenceCount;
};


//inline int SharedApproxData::num_variables() const
//{ return (dataRep) ? dataRep->numVars : numVars; }


inline void SharedApproxData::surrogate_data_index(size_t d_index)
{
  if (dataRep)
    dataRep->surrogate_data_index(d_index);
  else { // not virtual: all derived classes use following definition
    //if (d_index >= approxData.size()) {
    //  Cerr << "Error: index out of range in SharedApproxData::"
    //       << "surrogate_data_index()." << std::endl;
    //  abort_handler(APPROX_ERROR);
    //}
    activeDataIndex = d_index;
  }
}


inline void SharedApproxData::
set_bounds(const RealVector&  c_l_bnds, const RealVector&  c_u_bnds,
	   const IntVector&  di_l_bnds, const IntVector&  di_u_bnds,
	   const RealVector& dr_l_bnds, const RealVector& dr_u_bnds)
{
  // enforce deep copies since lower/upper may be active views
  if (dataRep) {
    copy_data(c_l_bnds,  dataRep->approxCLowerBnds);
    copy_data(c_u_bnds,  dataRep->approxCUpperBnds);
    copy_data(di_l_bnds, dataRep->approxDILowerBnds);
    copy_data(di_u_bnds, dataRep->approxDIUpperBnds);
    copy_data(dr_l_bnds, dataRep->approxDRLowerBnds);
    copy_data(dr_u_bnds, dataRep->approxDRUpperBnds);
  }
  else {
    copy_data(c_l_bnds,  approxCLowerBnds);
    copy_data(c_u_bnds,  approxCUpperBnds);
    copy_data(di_l_bnds, approxDILowerBnds);
    copy_data(di_u_bnds, approxDIUpperBnds);
    copy_data(dr_l_bnds, approxDRLowerBnds);
    copy_data(dr_u_bnds, approxDRUpperBnds);
  }
}


inline SharedApproxData* SharedApproxData::data_rep() const
{ return dataRep; }

} // namespace Dakota

#endif
