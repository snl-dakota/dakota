/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_QUADRATURE_H
#define NOND_QUADRATURE_H

#include "dakota_data_types.hpp"
#include "NonDIntegration.hpp"
#include "TensorProductDriver.hpp"

namespace Dakota {

// define special values for quadMode
enum { FULL_TENSOR, FILTERED_TENSOR, RANDOM_TENSOR };


/// Derived nondeterministic class that generates N-dimensional
/// numerical quadrature points for evaluation of expectation
/// integrals over uncorrelated standard
/// normals/uniforms/exponentials/betas/gammas.

/** This class is used by NonDPolynomialChaos, but could also be used
    for general numerical integration of moments.  It employs
    Gauss-Hermite, Gauss-Legendre, Gauss-Laguerre, Gauss-Jacobi and
    generalized Gauss-Laguerre quadrature for use with normal,
    uniform, exponential, beta, and gamma density functions and
    integration bounds.  The abscissas and weights for one-dimensional
    integration are extracted from the appropriate
    OrthogonalPolynomial class and are extended to n-dimensions using
    a tensor product approach. */

class NonDQuadrature: public NonDIntegration
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// alternate constructor for instantiations "on the fly" based on a
  /// quadrature order specification
  NonDQuadrature(Model& model, unsigned short quad_order,
		 const RealVector& dim_pref, short driver_mode);
  /// alternate constructor for instantiations "on the fly" that filter a
  /// tensor product sample set to include points with highest sample weights
  NonDQuadrature(Model& model, unsigned short quad_order,
		 const RealVector& dim_pref, short driver_mode,
		 int num_filt_samples);
  /// alternate constructor for instantiations "on the fly" that sub-sample
  /// quadrature rules by sampling randomly from a tensor product multi-index
  NonDQuadrature(Model& model, unsigned short quad_order,
		 const RealVector& dim_pref, short driver_mode,
		 int num_sub_samples, int seed);

  ~NonDQuadrature();                                       ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  void increment_grid();
  void decrement_grid();
  void evaluate_grid_increment();

  //
  //- Heading: Member functions
  //

  /// propagate any numSamples updates and/or grid updates/increments
  void update();

  /// set Pecos::TensorProductDriver::quadOrder to dimension orders indicated by
  /// quadOrderSpec & dimPrefSpec, following refinement or sequence advancement
  void reset();

  /// return Pecos::TensorProductDriver::quadOrder
  const Pecos::UShortArray& quadrature_order() const;
  /// set Pecos::TensorProductDriver::quadOrder
  void quadrature_order(const Pecos::UShortArray& dim_quad_order);
  /// set quadOrderSpec and map to Pecos::TensorProductDriver::quadOrder
  void quadrature_order(unsigned short quad_order);

  /// set numSamples
  void samples(size_t samples);

  /// return quadMode
  short mode() const;

protected:

  //
  //- Heading: Constructors and destructor
  //

  NonDQuadrature(ProblemDescDB& problem_db, Model& model); ///< constructor

  //
  //- Heading: Virtual function redefinitions
  //

  void initialize_grid(const std::vector<Pecos::BasisPolynomial>& poly_basis);

  void get_parameter_sets(Model& model);

  void sampling_reset(size_t min_samples, bool all_data_flag, bool stats_flag);
  void sampling_reference(size_t samples_ref);

  void increment_grid_preference(const RealVector& dim_pref);
  void increment_grid_preference();

  size_t num_samples() const;

  void random_seed(int seed);

private:

  //
  //- Heading: Convenience functions
  //

  /// convenience function used to make increment_grid() more modular
  void increment_grid(UShortArray& ref_quad_order);
  /// convenience function used to make increment_grid_preference() more modular
  void increment_grid_preference(const RealVector& dim_pref,
				 UShortArray& ref_quad_order);
  /// convenience function used to make decrement_grid() more modular
  void decrement_grid(UShortArray& ref_quad_order);

  /// calculate smallest dimension quadrature order with at least min_samples
  /// and propagate to Pecos::TensorProductDriver
  void compute_minimum_quadrature_order(size_t min_samples,
					const RealVector& dim_pref);

  /// prune allSamples back to size numSamples, retaining points
  /// with highest product weight
  void filter_parameter_sets();

  /// update quad_order_ref based on an updated dimension preference,
  /// enforcing previous values as a lower bound
  void update_anisotropic_order(const RealVector& dim_pref,
				UShortArray& quad_order_ref);

  /// initialize Pecos::TensorProductDriver::quadOrder from quad_order_spec
  /// and dim_pref_spec
  void initialize_dimension_quadrature_order(unsigned short quad_order_spec,
					     const RealVector& dim_pref_spec);

  /// increment each ref_quad_order entry by 1
  void increment_reference_quadrature_order(UShortArray& ref_quad_order);
  /// increment the ref_quad_order entry with maximum preference by 1
  /// and then rebalance
  void increment_reference_quadrature_order(const RealVector& dim_pref,
					    UShortArray& ref_quad_order);

  //
  //- Heading: Data
  //

  /// convenience pointer to the numIntDriver representation
  std::shared_ptr<Pecos::TensorProductDriver> tpqDriver;

  /// for studies involving refinement strategies, allow for use of nested
  /// quadrature rules such as Gauss-Patterson
  bool nestedRules;

  /// scalar quadrature order, rendered anisotropic via dimPrefSpec
  unsigned short quadOrderSpec;
  /// value of Pecos::TensorProductDriver::quadOrder prior to increment_grid(),
  /// for restoration in decrement_grid() (increment must induce a change in
  /// grid size and this increment may not be reversible).  Since this data is
  /// not keyed, increment/decrement must occur together prior to a key change.
  UShortArray refQuadOrderPrev;

  /// point generation mode: FULL_TENSOR, FILTERED_TENSOR, RANDOM_TENSOR
  short quadMode;
  /// size of a subset of tensor quadrature points (filtered based on product
  /// weight or sampled uniformly from the tensor multi-index); used by the
  /// regression PCE approach known as "probabilistic collocation"
  size_t numSamples;
  /// seed for the random number generator used in sampling of the tensor
  /// multi-index
  int randomSeed;
};


inline void NonDQuadrature::reset()
{
  // reset dimensional quadrature order to specification, prior to any grid
  // refinement, for the current active key
  // > updates to other keys are managed by {assign,increment}_specification_
  //   sequence() in multilevel expansion methods
  initialize_dimension_quadrature_order(quadOrderSpec, dimPrefSpec);
  // clear dist param update trackers
  tpqDriver->reset();
}


inline const Pecos::UShortArray& NonDQuadrature::quadrature_order() const
{ return tpqDriver->quadrature_order(); }


inline void NonDQuadrature::
quadrature_order(const Pecos::UShortArray& dim_quad_order)
{ tpqDriver->reference_quadrature_order(dim_quad_order, nestedRules); }


inline void NonDQuadrature::quadrature_order(unsigned short quad_order)
{ quadOrderSpec = quad_order; reset(); }


inline void NonDQuadrature::samples(size_t samples)
{
  switch (quadMode) {
  case FULL_TENSOR: {
    Cerr << "Error: setting samples not supported in FULL_TENSOR mode."
	 << std::endl;
    abort_handler(-1);
  }
  case FILTERED_TENSOR: case RANDOM_TENSOR:
    numSamples = samples; break;
  }
}


inline void NonDQuadrature::random_seed(int seed)
{ randomSeed = seed; }


inline void NonDQuadrature::update()
{
  switch (quadMode) {
  case FILTERED_TENSOR:
    // update settings and propagate to tpqDriver
    if (quadOrderSpec == USHRT_MAX)
      compute_minimum_quadrature_order(numSamples, dimPrefSpec);
    else
      reset();
    break;
  case RANDOM_TENSOR:
    // revise settings if needed to enforce min order
    // (in this context, this just reduces resampling)
    sampling_reset(numSamples, false, false);
    break;
  }
}


inline void NonDQuadrature::increment_grid()
{
  UShortArray ref_quad_order = tpqDriver->reference_quadrature_order(); // copy

  // cache refQuadOrderPrev for restoration in decrement_grid():
  // update both refQuadOrder and quadOrder in TensorProductDriver
  refQuadOrderPrev = ref_quad_order;

  increment_grid(ref_quad_order);
}


inline void NonDQuadrature::decrement_grid()
{
  // restore using refQuadOrderPrev from increment_grid() since the enforced
  // grid size increment is not reversible.  Restores both refQuadOrder and
  // quadOrder in TensorProductDriver, and nestedRules mapping is re-applied.
  tpqDriver->reference_quadrature_order(refQuadOrderPrev, nestedRules);
}


inline void NonDQuadrature::
increment_grid_preference(const RealVector& dim_pref) // update dimension pref
{
  UShortArray ref_quad_order = tpqDriver->reference_quadrature_order(); // copy

  // cache refQuadOrderPrev for restoration in decrement_grid():
  // enforced sample size increment is not reversible
  refQuadOrderPrev = ref_quad_order;

  increment_grid_preference(dim_pref, ref_quad_order);
}


inline void NonDQuadrature::increment_grid_preference()
{ increment_grid_preference(dimPrefSpec); } // no update to dimension preference


inline void NonDQuadrature::evaluate_grid_increment()
{
  // *** TO DO: implement incremental build in Pecos::TensorProductDriver
  // based on webbur::point_radial_tol_unique_index_inc2(), as in Pecos::
  // IncrementalSparseGridDriver::increment_unique().
  // (Relying on duplicate detection as below is insufficient for rebuilds
  // since the point counts in latest incremental logic are wrong...)
  //
  // Note: this would require introduction of collocation indices into
  // TensorProductDriver as the incremental evaluations in an updated grid
  // would no longer be in tensor order.  For this reason, rely on duplication
  // detection for now.

  tpqDriver->compute_grid(allSamples);//Driver->compute_increment(allSamples);
  evaluate_parameter_sets(iteratedModel, true, false);
  ++numIntegrations;
}


inline void NonDQuadrature::sampling_reference(size_t samples_ref)
{ numSamples = samples_ref; }


inline size_t NonDQuadrature::num_samples() const
{
  switch (quadMode) {
  case FULL_TENSOR:                        return tpqDriver->grid_size(); break;
  case FILTERED_TENSOR: case RANDOM_TENSOR:            return numSamples; break;
  }
  return 0; // should not happen
}


inline short NonDQuadrature::mode() const
{ return quadMode; }

} // namespace Dakota

#endif
