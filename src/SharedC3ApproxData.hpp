/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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

  // *** IMPORTANT NOTE: these regression_size() implementations utilize shared
  // *** before-build input specification (subject to increment/decrement), not
  // *** after-build per-QoI details (subject to CV adapt_rank/adapt_order)

  /// return number of FT unknowns given scalars: num vars, rank, order
  static size_t regression_size(size_t num_v, size_t rank, size_t max_rank,
				const UShortArray& orders,
				unsigned short max_order);
  /// return number of FT unknowns using start_rank(), max_rank(),
  /// start_orders(), max_order()
  size_t regression_size();
  /// return number of FT unknowns using maximum rank, start_orders(),
  /// max_order()
  size_t max_rank_regression_size();
  /// return number of FT unknowns using start_rank(), max_rank(), and
  /// maximum basis order
  size_t max_order_regression_size();
  /// return number of FT unknowns using maxima for rank and basis order
  size_t max_regression_size();

  /// set UShortArray attribute value based on identifier string
  void set_parameter(String var, const UShortArray& val);
  /// set unsigned short attribute value based on identifier string
  void set_parameter(String var, unsigned short     val);
  /// set size_t attribute value based on identifier string
  void set_parameter(String var, size_t val);
  /// set bool attribute value based on identifier string
  void set_parameter(String var, bool   val);
  /// set short attribute value based on identifier string
  void set_parameter(String var, short  val);
  /// set double attribute value based on identifier string
  void set_parameter(String var, double val);
  /// set int attribute value based on identifier string
  void set_parameter(String var, int    val);

  /// set active UShortArray attribute value based on identifier string
  void set_active_parameter(String var, const UShortArray& val);
  /// set active unsigned short attribute value based on identifier string
  void set_active_parameter(String var, unsigned short val);
  /// set active size_t attribute value based on identifier string
  void set_active_parameter(String var, size_t val);
  /// set active int attribute value based on identifier string
  void set_active_parameter(String var, int val);

  // set SharedOrthogPolyApproxData::basisTypes
  //void basis_types(const ShortArray& basis_types);
  // get SharedOrthogPolyApproxData::basisTypes
  //const ShortArray& basis_types() const;

  /// return active start value for basis order
  const UShortArray& start_orders() const;
  /// return active start value for basis order (mutable)
  UShortArray& start_orders();
  /// return active maximum value for basis order
  unsigned short max_order() const;
  /// return active maximum value for basis order (mutable)
  unsigned short& max_order();

  /// return active start value for expansion rank
  size_t start_rank() const;
  /// return active start value for expansion rank (mutable)
  size_t& start_rank();
  /// return active maximum value for expansion rank
  size_t max_rank() const;
  /// return active maximum value for expansion rank (mutable)
  size_t& max_rank();

  /// return maxCVRankCandidates
  size_t max_cross_validation_rank_candidates() const;
  /// return maxCVOrderCandidates
  unsigned short max_cross_validation_order_candidates() const;
  // infer maxCV{Rank,Order}Candidates from user spec
  //void infer_max_cross_validation_ranges();

  void assign_start_ranks(SizetVector& start_ranks) const;

  // return c3AdvancementType
  //short advancement_type() const;

  /// update oneApproxOpts with active basis orders after an order change
  void update_basis();
  /// update oneApproxOpts with passed basis orders after an order change
  void update_basis(const UShortArray& start_orders, unsigned short max_order);
  /// update oneApproxOpts for variable v with passed basis orders
  void update_basis(size_t v, unsigned short start_order,
		    unsigned short max_order);

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void active_model_key(const Pecos::ActiveKey& key);
  //void clear_model_keys();

  void construct_basis(const Pecos::MultivariateDistribution& mv_dist);

  short discrepancy_reduction() const;

  // Activate as additional features (and basis types) come online
  
  //void update_basis_distribution_parameters(        // beyond STD_{NORMAL,
  //  const Pecos::MultivariateDistribution& u_dist); //             UNIFORM}

  //void configuration_options(const Pecos::ExpansionConfigOptions& ec_options);
  //void configuration_options(const Pecos::BasisConfigOptions&     bc_options);

  void random_variables_key(const BitArray& random_vars_key);

  //void refinement_statistics_mode(short stats_mode);

  //const Pecos::BitArrayULongMap& SharedApproxData::sobol_index_map();

  void build();
  //void rebuild(); // defaults to build()

  void increment_order();
  void decrement_order();

  void   pop(bool save_surr_data);
  bool   push_available();
  size_t push_index(const Pecos::ActiveKey& key);
  void   post_push();

  void pre_combine();
  //void post_combine();

  bool advancement_available();
  void max_rank_advancement(bool r_advance);
  void max_order_advancement(bool o_advance);

  //
  //- Heading: Convenience functions
  //

  bool increment_max_rank();
  bool increment_max_order();
  //bool decrement_start_rank();
  //bool decrement_start_order();

  //
  //- Heading: Data
  //

  // While these containers are shared across the QoI, some data now varies per
  // QoI (for adapt_order). Per QoI data is pushed/pulled in C3Approx::build().
  //
  // lower level basis options
  //std::vector<OpeOpts*> polyBasisOpts;
  /// one-D approximation options (basis type, poly order, etc.)
  std::vector<OneApproxOpts*> oneApproxOpts;
  /// n-D approximation options, augmenting one-D options
  MultiApproxOpts* multiApproxOpts;

  /// option to scale response data prior to regression
  bool respScaling;

  /// starting user specification for polynomial orders (from start_order
  /// scalar plus anisotropic dimension preference)
  UShortArray startOrders;
  /// starting values for polynomial order (prior to adaptive refinement);
  /// for each model key, there is an array of polynomial orders per variable
  std::map<Pecos::ActiveKey, UShortArray> startOrdersMap;
  /// user specification for increment in order used within adapt_order
  unsigned short kickOrder;
  /// maximum value for polynomial order from user spec
  unsigned short maxOrder;
  /// user specification for maximum order used within adapt_order;
  /// usually a scalar specification but can be adapted per model key
  /// for MAX_{ORDER,RANK_ORDER}_ADVANCEMENT refine types
  std::map<Pecos::ActiveKey, unsigned short> maxOrderMap;
  /// C3 FT can support CV over polynomial order in addition to adapt_rank
  bool adaptOrder;

  /// polynomial basis order for combined expansion for each variable core
  UShortArray combinedOrders;

  // the remaining data are separate as can be seen in C3Approximation::build()

  /// starting user specification for rank (not augmented by dimension
  /// preference); Note: rank sequence spec is managed externally and becomes
  /// reflected in startRank model index mapping
  size_t startRank; // or SizetArray for start_rank_sequence spec
  /// starting values for rank (note: adapt_rank currently covers refinement);
  /// for each model index key, there is a scalar starting rank (recovered
  /// rank in C3FnrainPtrs can vary per core/variable and per QoI)
  std::map<Pecos::ActiveKey, size_t> startRankMap;
  /// user specification for increment in rank used within adapt_rank
  size_t kickRank;
  /// scalar user specification for maximum allowable rank when adapting
  size_t maxRank;
  /// user specification for maximum rank used within adapt_rank;
  /// usually a scalar specification but can be adapted per model key
  /// for MAX_{RANK,RANK_ORDER}_ADVANCEMENT refine types
  std::map<Pecos::ActiveKey, size_t> maxRankMap;
  /// internal C3 adaptation that identifies the best rank representation
  /// for a set of sample data based on cross validation
  bool adaptRank;

  /// type of regression solver for forming FT approximation
  short  regressType;
  /// penalty parameter if regularized regression
  double regressRegParam;
  /// tolerance on regression solver
  double solverTol;
  /// tolerance for rounding (performing a truncation operation on a FT
  /// expansion) within the regression solver
  double solverRoundingTol;
  /// tolerance for rounding (performing a truncation operation on a FT
  /// expansion) when post-processing an expansion: computing products
  /// for moments, combining expansions with c3axpy, etc.
  double statsRoundingTol;
  /// maximum number of iterations for regression solver
  size_t maxSolverIterations;
  /// maximum number of iterations for (future) cross iteration solver
  int crossMaxIter;
  /// C3 regression solver employs a random seed
  int randomSeed;

  // a more general adaptive construction option, distinct from adapt_rank
  //bool adaptConstruct; // inactive placeholder for now

  /// type of discrepancy calculation: additive, multiplicative, or both
  short combineType;
  /// type of multilevel discrepancy emulation: distinct or recursive
  short discrepReduction;
  /// type of multilevel strategy for sample allocation: ESTIMATOR_VARIANCE,
  /// RANK_SAMPLING, GREEDY
  short allocControl;
  // indicates refinement based on active or combined statistics
  //short refineStatsMode;
  /// type of advancement strategy used in uniform refinement:
  /// {START_ORDER,START_RANK,MAX_ORDER,MAX_RANK,MAX_RANK_ORDER}_ADVANCEMENT
  short c3AdvancementType;

  /// flag indicating availability of rank advancement (accumulated from
  /// C3Approximation::advancement_available())
  std::map<Pecos::ActiveKey, bool> c3MaxRankAdvance;
  /// flag indicating availability of order advancement (accumulated from
  /// C3Approximation::advancement_available())
  std::map<Pecos::ActiveKey, bool> c3MaxOrderAdvance;

  /// restrict the number of candidates within cross validation for order
  /// (by increasing start order when needed as max order is advanced)
  unsigned short maxCVOrderCandidates;
  /// restrict the number of candidates within cross validation for rank
  /// (by increasing start rank when needed as max rank is advanced)
  size_t maxCVRankCandidates;

  // key identifying the subset of build variables that can be treated
  // as random, for purposes of computing statistics
  //BitArray ranVarsKey; // stored locally rather than passed to library (Pecos)
  /// indices for random subset when approximating in all-variables mode
  SizetArray randomIndices;

  /// number of instances within the popped arrays (mostly a placeholder for
  /// supporting push_available())
  std::map<Pecos::ActiveKey, size_t> poppedCounts;

private:

  //
  //- Heading: Data
  //

};


inline SharedC3ApproxData::SharedC3ApproxData()
{ }


inline void SharedC3ApproxData::active_model_key(const Pecos::ActiveKey& key)
{
  // set activeKey
  SharedApproxData::active_model_key(key);

  // these aren't used enough to warrant active iterators
  bool form = false;
  if (startOrdersMap.find(key) == startOrdersMap.end())
    { startOrdersMap[key]      =  startOrders;  form = true; }
  if (startRankMap.find(key)   == startRankMap.end()) 
    { startRankMap[key]        =  startRank;    form = true; }
  if (maxOrderMap.find(key)    == maxOrderMap.end())
    { maxOrderMap[key]         =  maxOrder;     if (adaptOrder) form = true; }
  if (maxRankMap.find(key)     == maxRankMap.end())
    { maxRankMap[key]          =  maxRank;      if (adaptRank)  form = true; }

  // ensure approximation rebuild, when needed, in absence of sample increment
  if (form) formUpdated[key] = true;
}


inline void SharedC3ApproxData::
assign_start_ranks(SizetVector& start_ranks) const
{
  size_t v, num_vp1 = numVars + 1, start_r = start_rank();
  if (start_ranks.length() != num_vp1)
    start_ranks.sizeUninitialized(num_vp1);
  start_ranks[0] = start_ranks[numVars] = 1;
  for (v=1; v<numVars; ++v) start_ranks[v] = start_r;
}


inline void SharedC3ApproxData::update_basis()
{ update_basis(start_orders(), max_order()); }


inline short SharedC3ApproxData::discrepancy_reduction() const
{ return discrepReduction; }


inline const UShortArray& SharedC3ApproxData::start_orders() const
{
  std::map<Pecos::ActiveKey, UShortArray>::const_iterator cit
    = startOrdersMap.find(activeKey);
  return (cit == startOrdersMap.end()) ? startOrders : cit->second;
}


inline UShortArray& SharedC3ApproxData::start_orders()
{
  std::map<Pecos::ActiveKey, UShortArray>::iterator it
    = startOrdersMap.find(activeKey);
  return (it == startOrdersMap.end()) ? startOrders : it->second;
}


inline unsigned short SharedC3ApproxData::max_order() const
{
  std::map<Pecos::ActiveKey, unsigned short>::const_iterator cit
    = maxOrderMap.find(activeKey);
  return (cit == maxOrderMap.end()) ? maxOrder : cit->second;
}


inline unsigned short& SharedC3ApproxData::max_order()
{
  std::map<Pecos::ActiveKey, unsigned short>::iterator it
    = maxOrderMap.find(activeKey);
  return (it == maxOrderMap.end()) ? maxOrder : it->second;
}


inline size_t SharedC3ApproxData::start_rank() const
{
  std::map<Pecos::ActiveKey, size_t>::const_iterator cit
    = startRankMap.find(activeKey);
  return (cit == startRankMap.end()) ? startRank : cit->second;
}


inline size_t& SharedC3ApproxData::start_rank()
{
  std::map<Pecos::ActiveKey, size_t>::iterator it
    = startRankMap.find(activeKey);
  return (it == startRankMap.end()) ? startRank : it->second;
}


inline size_t SharedC3ApproxData::max_rank() const
{
  std::map<Pecos::ActiveKey, size_t>::const_iterator cit
    = maxRankMap.find(activeKey);
  return (cit == maxRankMap.end()) ? maxRank : cit->second;
}


inline size_t& SharedC3ApproxData::max_rank()
{
  std::map<Pecos::ActiveKey, size_t>::iterator it = maxRankMap.find(activeKey);
  return (it == maxRankMap.end()) ? maxRank : it->second;
}


inline size_t SharedC3ApproxData::max_cross_validation_rank_candidates() const
{ return maxCVRankCandidates; }


inline unsigned short SharedC3ApproxData::
max_cross_validation_order_candidates() const
{ return maxCVOrderCandidates; }


//inline short SharedC3ApproxData::advancement_type() const
//{ return c3AdvancementType; }


/** simplified estimation for scalar-valued rank and order (e.g., from 
    start rank/order user specification)
inline size_t SharedC3ApproxData::
regression_size(size_t num_v, size_t rank, size_t order)
{
  // Each dimension has its own rank within the product of function cores.
  // This fn estimates for the case where rank and order are either constant
  // across dimensions or averaged into a scalar.
  // > the first and last core contribute p*r terms
  // > the middle cores contribute r*r*p terms
  size_t p = order+1.;
  switch (num_v) {
  case 1:  return p;         break; // collapses to a 1D PCE
  case 2:  return 2.*p*rank; break; // first and last core, no middle
  default: return p*rank*(2. + (num_v-2)*rank); break; // first,last,middle
  }
}
*/


/** simplified estimation for scalar-valued rank and vector-valued order
    (e.g., from start rank/start order/dimension pref user specification) */
inline size_t SharedC3ApproxData::
regression_size(size_t num_v, size_t rank, size_t max_rank,
		const UShortArray& orders, unsigned short max_order)
{
  // Each dimension has its own rank within the product of function cores.
  // This fn estimates for the case where rank and order are either constant
  // across dimensions or averaged into a scalar.
  // > the first and last core contribute p*r terms
  // > the middle cores contribute r*r*p terms
  unsigned short p;
  switch (num_v) {
  case 1:
    p = std::min(orders[0], max_order) + 1;
    return p; break; // collapses to 1D PCE
  default: { // first, last, and num_v-2 middle cores
    size_t core, num_vm1 = num_v - 1, sum, r = std::min(rank, max_rank);
    p = std::min(orders[0],       max_order) + 1;  sum  = p; // first
    p = std::min(orders[num_vm1], max_order) + 1;  sum += p; // last
    for (core=1; core<num_vm1; ++core) {
      p = std::min(orders[core], max_order) + 1;
      sum += r * p;  // num_v-2 middle cores
    }
    return sum * r;  break;
  }
  }
}


inline size_t SharedC3ApproxData::regression_size()
{
  return regression_size(numVars, start_rank(), max_rank(),
			 start_orders(), max_order());

  /* If max regression size logic were proliferated to non-refinement cases,
     could automate max helpers below based on adapt{Rank,Order}:
  size_t max_r = max_rank(), regress_r = (adaptRank) ? max_r : start_rank();
  unsigned short max_o = max_order();
  if (adaptOrder) {
    UShortArray max_orders(numVars, max_o);// no dim_pref; adapt is isotropic
    return regression_size(numVars, regress_r, max_r,   max_orders,   max_o);
  }
  else
    return regression_size(numVars, regress_r, max_r, start_orders(), max_o);
  */
}


inline size_t SharedC3ApproxData::max_rank_regression_size()
{
  size_t max_r = max_rank();
  return regression_size(numVars, max_r, max_r, start_orders(), max_order());
}


inline size_t SharedC3ApproxData::max_order_regression_size()
{
  //UShortArray max_orders; RealVector dim_pref;//isotropic for now: no XML spec
  //Pecos::dimension_preference_to_anisotropic_order(max_o, dim_pref,
  //  numVars, max_orders);

  unsigned short max_o = max_order();
  UShortArray max_orders(numVars, max_o);// no dim_pref; adaptOrder is isotropic

  return regression_size(numVars, start_rank(), max_rank(), max_orders, max_o);
}


inline size_t SharedC3ApproxData::max_regression_size()
{
  //UShortArray max_orders; RealVector dim_pref;//isotropic for now: no XML spec
  //Pecos::dimension_preference_to_anisotropic_order(max_o, dim_pref,
  //  numVars, max_orders);

  size_t max_r = max_rank();  unsigned short max_o = max_order();
  UShortArray max_orders(numVars, max_o);// no dim_pref; adaptOrder is isotropic

  return regression_size(numVars, max_r, max_r, max_orders, max_o);
}


inline void SharedC3ApproxData::max_rank_advancement(bool r_advance)
{ c3MaxRankAdvance[activeKey] = r_advance; }


inline void SharedC3ApproxData::max_order_advancement(bool o_advance)
{ c3MaxOrderAdvance[activeKey] = o_advance; }


inline void SharedC3ApproxData::
set_parameter(String var, const UShortArray& val)
{
  if (var.compare("start_order") == 0)         startOrders = val;
  else Cerr << "Unrecognized C3 parameter: " << var << std::endl;
}


inline void SharedC3ApproxData::set_parameter(String var, unsigned short val)
{
  if (var.compare("kick_order")     == 0)               kickOrder = val;
  else if (var.compare("max_order") == 0)                maxOrder = val;
  else if (var.compare("max_cv_order") == 0) maxCVOrderCandidates = val;
  else Cerr << "Unrecognized C3 parameter: " << var << std::endl;
}


inline void SharedC3ApproxData::set_parameter(String var, size_t val)
{
  if (var.compare("start_rank")       == 0)                     startRank = val;
  else if (var.compare("kick_rank")   == 0)                      kickRank = val;
  else if (var.compare("max_rank")    == 0)                       maxRank = val;
  else if (var.compare("max_cv_rank") == 0)           maxCVRankCandidates = val;
  else if (var.compare("max_solver_iterations") == 0) maxSolverIterations = val;
  else Cerr << "Unrecognized C3 parameter: " << var << std::endl;
}


inline void SharedC3ApproxData::set_parameter(String var, bool val)
{
  if (var.compare("adapt_rank") == 0)             adaptRank  = val;
  else if (var.compare("adapt_order") == 0)       adaptOrder = val;
  else if (var.compare("response_scaling") == 0) respScaling = val;
  else Cerr << "Unrecognized C3 parameter: " << var << std::endl;
}


inline void SharedC3ApproxData::set_parameter(String var, short val)
{
  if      (var.compare("regress_type")      == 0)       regressType = val;
  else if (var.compare("discrepancy_type")  == 0)  discrepReduction = val;
  else if (var.compare("alloc_control")     == 0)      allocControl = val;
  else if (var.compare("combine_type")      == 0)       combineType = val;
  else if (var.compare("advancement_type")  == 0) c3AdvancementType = val;
  else Cerr << "Unrecognized C3 parameter: " << var << std::endl;
}


inline void SharedC3ApproxData::set_parameter(String var, double val)
{
  if      (var.compare("solver_tol")          == 0)            solverTol = val;
  else if (var.compare("solver_rounding_tol") == 0)    solverRoundingTol = val;
  else if (var.compare("stats_rounding_tol")  == 0)     statsRoundingTol = val;
  else if (var.compare("regularization_parameter") == 0) regressRegParam = val;
  else Cerr << "Unrecognized C3 parameter: " << var << std::endl;
}


inline void SharedC3ApproxData::set_parameter(String var, int val)
{
  if      (var.compare("max_cross_iterations")  == 0) crossMaxIter = val;
  else if (var.compare("random_seed")           == 0)   randomSeed = val;
  else Cerr << "Unrecognized C3 parameter: " << var << std::endl;
}


inline void SharedC3ApproxData::
set_active_parameter(String var, const UShortArray& val)
{
  if (var.compare("start_order") == 0)
    { startOrdersMap[activeKey] = val; /*update_basis();*/ }
  else Cerr << "Unrecognized C3 active parameter: " << var << std::endl;
}


inline void SharedC3ApproxData::
set_active_parameter(String var, unsigned short val)
{
  if (var.compare("max_order") == 0)
    { maxOrderMap[activeKey] = val; /*if (adaptOrder) update_basis();*/ }
  else Cerr << "Unrecognized C3 active parameter: " << var << std::endl;
}


inline void SharedC3ApproxData::set_active_parameter(String var, size_t val)
{
  if      (var.compare("start_rank") == 0) startRankMap[activeKey] = val;
  else if (var.compare("max_rank")   == 0)   maxRankMap[activeKey] = val;
  else Cerr << "Unrecognized C3 active parameter: " << var << std::endl;
}


inline void SharedC3ApproxData::set_active_parameter(String var, int val)
{
  if (var.compare("random_seed") == 0) randomSeed = val;
  else Cerr << "Unrecognized C3 active parameter: " << var << std::endl;
}


inline void SharedC3ApproxData::
random_variables_key(const BitArray& random_vars_key)
{
  // store incoming mask:
  //ranVarsKey = random_vars_key;

  // convert incoming mask to a set of random indices:
  if (random_vars_key.empty()) {
    randomIndices.resize(numVars);
    for (size_t i=0; i<numVars; ++i)
      randomIndices[i] = i;
  }
  else {
    // NonDExpansion::initialize_u_space_model() assigns startCAUV,numCAUV
    // subset within numContinuousVars
    size_t i, cntr, num_rand = random_vars_key.count();
    randomIndices.resize(num_rand);
    for (i=0, cntr=0; i<numVars; ++i)
      if (random_vars_key[i])
	randomIndices[cntr++] = i;
  }
}


//inline void SharedC3ApproxData::refinement_statistics_mode(short stats_mode)
//{ refineStatsMode = stats_mode; }


inline void SharedC3ApproxData::build()
{
  // Ideally this would occur in post_build(), but for now there is only a
  // SharedApproxData::build() that precedes Approximation::build()

  // formulation updates (rank,order increments/decrements) have been
  // synchronized once build is complete (no longer need to force a rebuild):
  formUpdated[activeKey] = false;
}


inline void SharedC3ApproxData::pop(bool save_surr_data)
{
  std::map<Pecos::ActiveKey, size_t>::iterator it
    = poppedCounts.find(activeKey);
  if (it == poppedCounts.end())
    poppedCounts[activeKey] = 1;
  else
    ++it->second; // increment count
}


inline bool SharedC3ApproxData::push_available()
{
  std::map<Pecos::ActiveKey, size_t>::iterator it
    = poppedCounts.find(activeKey);
  return (it != poppedCounts.end() && it->second > 0);
}


inline size_t SharedC3ApproxData::push_index(const Pecos::ActiveKey& key)
{
  std::map<Pecos::ActiveKey, size_t>::iterator it = poppedCounts.find(key);
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
  std::map<Pecos::ActiveKey, size_t>::iterator it
    = poppedCounts.find(activeKey);
  if (it != poppedCounts.end() && it->second > 0)
    --it->second; // decrement count
}

} // namespace Dakota

#endif
