/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SharedC3ApproxData
//- Description:  Base Class for C3 aprpoximation interface
//-               
//- Owner:        Alex Gorodetsky

#ifndef SHARED_C3_APPROX_DATA_H
#define SHARED_C3_APPROX_DATA_H

#include "SharedApproxData.hpp"
#include "SharedRegressOrthogPolyApproxData.hpp"
#include "dakota_c3_include.hpp"

namespace Dakota {


/// Derived approximation class for global basis polynomials.

/** The SharedC3ApproxData class provides a global approximation
    based on basis polynomials.  This includes orthogonal polynomials
    used for polynomial chaos expansions and interpolation polynomials
    used for stochastic collocation. */

class SharedC3ApproxData: public SharedApproxData
{
  //
  //- Heading: Friends
  //

  friend class C3Approximation;

public:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor
  SharedC3ApproxData();
  /// standard ProblemDescDB-driven constructor
  SharedC3ApproxData(ProblemDescDB& problem_db, size_t num_vars);
  /// on-the-fly constructor (no problem DB)
  SharedC3ApproxData(const String& approx_type,
                     const UShortArray& approx_order, size_t num_vars,
                     short data_order, short output_level);
  /// destructor
  ~SharedC3ApproxData();

  //
  //- Heading: Member functions
  //

  size_t pre_combine(short);
  void post_combine(short);

  void construct_basis(const Pecos::MultivariateDistribution& u_dist);

  void set_parameter(String var, size_t val);
  void set_parameter(String var, bool   val);
  void set_parameter(String var, short  val);
  void set_parameter(String var, double val);
  void set_parameter(String var, int    val);

  // set SharedOrthogPolyApproxData::basisTypes
  //void basis_types(const ShortArray& basis_types);
  // get SharedOrthogPolyApproxData::basisTypes
  //const ShortArray& basis_types() const;

  /// return current basis polynomial order (startOrder for now)
  size_t polynomial_order() const;

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  //void clear_model_keys();

  short discrepancy_type() const;

  // Activate as additional features (and basis types) come online
  
  //void update_basis_distribution_parameters(        // beyond STD_{NORMAL,
  //  const Pecos::MultivariateDistribution& u_dist); //             UNIFORM}

  //void configuration_options(const Pecos::ExpansionConfigOptions& ec_options);
  //void configuration_options(const Pecos::BasisConfigOptions&     bc_options);

  void random_variables_key(const BitArray& random_vars_key);

  //void refinement_statistics_type(short stats_type);
  //const Pecos::BitArrayULongMap& SharedApproxData::sobol_index_map();

  //void build();
  //void rebuild();
  
  void   pop(bool save_surr_data);
  bool   push_available();
  size_t push_index(const UShortArray& key);
  void   post_push();

  //
  //- Heading: Data
  //

  // This data is shared per QoI _and_ per level
  //
  /// 1D approximation options (basis type, poly order, etc.)
  struct OneApproxOpts ** oneApproxOpts;
  /// n-D approximation options, augmenting 1D options
  struct MultiApproxOpts * approxOpts;

  // these are stored in oneApproxOpts, but currently need to be cached
  // to persist between set_parameter() and construct_basis()
  //
  /// starting point for polynomial order (not currently adapted for regression)
  size_t startOrder; // Sequence?
  /// maximum value for polynomial order (if adapted)
  size_t maxOrder;

  // the remaining data are separate as can be seen in C3Approximation::build()

  size_t startRank; // Sequence?
  size_t  kickRank;
  size_t   maxRank;
  bool   adaptRank; // converted to size_t (0 no, 1 yes) for C3

  short  regressType;
  double regressRegParam; // penalty parameter if regularized regression
  double roundingTol;
  double solverTol;
  int maxSolverIterations;
  int crossMaxIter;
  int c3Verbosity;

  /// a more general adaptive construction option, distinct from adapt_rank
  bool adaptConstruct; // inactive placeholder for now
  /// C3 FT can support CV over polynomial order in addition to adapt_rank
  bool crossVal;       // inactive placeholder for now

  /// type of discrepancy calculation: additive, multiplicative, or both
  short combineType;
  /// type of multilevel discrepancy emulation: distinct or recursive
  short discrepancyType;
  /// type of multilevel strategy for sample allocation: ESTIMATOR_VARIANCE,
  /// RANK_SAMPLING, GREEDY
  short allocControl;

  // key identifying the subset of build variables that can be treated
  // as random, for purposes of computing statistics
  //BitArray ranVarsKey; // stored locally rather than passed to library (Pecos)
  /// indices for random subset when approximating in all-variables mode
  SizetVector randomIndices;

  /// number of instances within the popped arrays (mostly a placeholder for
  /// supporting push_available())
  std::map<UShortArray, size_t> poppedCounts;

private:

  //
  //- Heading: Convenience functions
  //

  //
  //- Heading: Data
  //

};


inline SharedC3ApproxData::SharedC3ApproxData()
{ }


inline short SharedC3ApproxData::discrepancy_type() const
{ return discrepancyType; }


inline size_t SharedC3ApproxData::polynomial_order() const
{ return startOrder; }


inline void SharedC3ApproxData::set_parameter(String var, size_t val)
{
  if      (var.compare("start_poly_order") == 0) startOrder = val;
  else if (var.compare("max_poly_order")   == 0)   maxOrder = val;
  else if (var.compare("start_rank")       == 0)  startRank = val;
  else if (var.compare("kick_rank")        == 0)   kickRank = val;
  else if (var.compare("max_rank")         == 0)    maxRank = val;
  else std::cerr << "Unrecognized C3 parameter: " << var << std::endl;
}


inline void SharedC3ApproxData::set_parameter(String var, bool val)
{
  if (var.compare("adapt_rank") == 0)  adaptRank = val;
  else std::cerr << "Unrecognized C3 parameter: " << var << std::endl;
}


inline void SharedC3ApproxData::set_parameter(String var, short val)
{
  if      (var.compare("regress_type")     == 0)     regressType = val;
  else if (var.compare("discrepancy_type") == 0) discrepancyType = val;
  else if (var.compare("alloc_control")    == 0)    allocControl = val;
  else if (var.compare("combine_type")     == 0)     combineType = val;
  else std::cerr << "Unrecognized C3 parameter: " << var << std::endl;
}


inline void SharedC3ApproxData::set_parameter(String var, double val)
{
  if      (var.compare("solver_tol")               == 0)       solverTol = val;
  else if (var.compare("rounding_tol")             == 0)     roundingTol = val;
  else if (var.compare("regularization_parameter") == 0) regressRegParam = val;
  else std::cerr << "Unrecognized C3 parameter: " << var << std::endl;
}


inline void SharedC3ApproxData::set_parameter(String var, int val)
{
  if      (var.compare("max_cross_iterations")  == 0)        crossMaxIter = val;
  else if (var.compare("max_solver_iterations") == 0) maxSolverIterations = val;
  else if (var.compare("verbosity")             == 0)         c3Verbosity = val;
  else std::cerr << "Unrecognized C3 parameter: " << var << std::endl;
}


inline void SharedC3ApproxData::
random_variables_key(const BitArray& random_vars_key)
{
  // store incoming mask:
  //ranVarsKey = random_vars_key;

  // convert incoming mask to a set of random indices:
  if (random_vars_key.empty()) {
    randomIndices.sizeUninitialized(numVars);
    for (size_t i=0; i<numVars; ++i)
      randomIndices[i] = i;
  }
  else {
    size_t i, cntr, num_rand = random_vars_key.count();
    randomIndices.sizeUninitialized(num_rand);
    for (i=0, cntr=0; i<numVars; ++i)
      if (random_vars_key[i])
	randomIndices[cntr++] = i;
  }
}


inline void SharedC3ApproxData::pop(bool save_surr_data)
{
  std::map<UShortArray, size_t>::iterator it = poppedCounts.find(activeKey);
  if (it == poppedCounts.end())
    poppedCounts[activeKey] = 1;
  else
    ++it->second; // increment count
}


inline bool SharedC3ApproxData::push_available()
{
  std::map<UShortArray, size_t>::iterator it = poppedCounts.find(activeKey);
  return (it != poppedCounts.end() && it->second > 0);
}


inline size_t SharedC3ApproxData::push_index(const UShortArray& key)
{
  std::map<UShortArray, size_t>::iterator it = poppedCounts.find(key);
  if (it == poppedCounts.end() || it->second == 0) {
    Cerr << "Error: bad lookup in SharedC3ApproxData::push_index(key)"
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }
  // there should only be a single refinement candidate per level (as for
  // uniform refinement cases in Pecos), but this is more general:
  else
    return it->second - 1; // last in, first out
}


inline void SharedC3ApproxData::post_push()
{
  std::map<UShortArray, size_t>::iterator it = poppedCounts.find(activeKey);
  if (it != poppedCounts.end() && it->second > 0)
    --it->second; // decrement count
}

} // namespace Dakota

#endif
