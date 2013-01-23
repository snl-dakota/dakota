/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDSparseGrid
//- Description: Implementation code for NonDSparseGrid class
//- Owner:       Mike Eldred
//- Revised by:  
//- Version:

#include "dakota_data_io.hpp"
#include "dakota_system_defs.hpp"
#include "NonDSparseGrid.hpp"
#include "DakotaModel.hpp"
#include "ProblemDescDB.hpp"
#include "PolynomialApproximation.hpp"

static const char rcsId[]="@(#) $Id: NonDSparseGrid.cpp,v 1.57 2004/06/21 19:57:32 mseldre Exp $";

//#define DEBUG

namespace Dakota {


/** This constructor is called for a standard letter-envelope iterator
    instantiation.  In this case, set_db_list_nodes has been called
    and probDescDB can be queried for settings from the method
    specification.  It is not currently used, as there is not a
    separate sparse_grid method specification. */
NonDSparseGrid::NonDSparseGrid(Model& model): NonDIntegration(model),
  ssgLevelSeqSpec(probDescDB.get_usa("method.nond.sparse_grid_level")),
  ssgLevelRef(ssgLevelSeqSpec[sequenceIndex])
{
  // initialize the numerical integration driver
  short exp_coeffs_approach =
    //problemDescDB.get_short("method.nond.sparse_grid_type")
    Pecos::COMBINED_SPARSE_GRID;
  numIntDriver = Pecos::IntegrationDriver(exp_coeffs_approach);
  ssgDriver = (Pecos::SparseGridDriver*)numIntDriver.driver_rep();

  // initialize_random_variables() called in NonDIntegration ctor
  check_variables(natafTransform.x_types());

  short refine_type
    = probDescDB.get_short("method.nond.expansion_refinement_type");
  short refine_control
    = probDescDB.get_short("method.nond.expansion_refinement_control");
  short growth_override = probDescDB.get_short("method.nond.growth_override");
  bool use_derivs = probDescDB.get_bool("method.derivative_usage");

  bool store_colloc = false; // no collocIndices/gauss{Pts,Wts}1D storage
  bool nested_rules = (probDescDB.get_short("method.nond.nesting_override")
		       != Pecos::NON_NESTED);
  bool track_uniq_prod_wts = false;
  bool track_colloc_indices
    = (exp_coeffs_approach == Pecos::COMBINED_SPARSE_GRID);
  bool piecewise_basis = (probDescDB.get_bool("method.nond.piecewise_basis") ||
			  refine_type == Pecos::H_REFINEMENT);
  bool equidist_rules = true; // NEWTON_COTES pts for piecewise interpolants
  // moderate growth is helpful for iso and aniso sparse grids, but
  // not necessary for generalized grids
  //short refine_type
  //  = probDescDB.get_short("method.nond.expansion_refinement_type");
  short growth_rate;
  if (growth_override == Pecos::UNRESTRICTED ||
      refine_control  == Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED)
    // unstructured index set evolution: no motivation to restrict
    growth_rate = Pecos::UNRESTRICTED_GROWTH;
  else if (piecewise_basis)
    // no reason to match Gaussian precision, but restriction still useful:
    // use SLOW i=2l+1 since it is more natural for NEWTON_COTES,CLENSHAW_CURTIS
    // and is more consistent with UNRESTRICTED generalized sparse grids.
    growth_rate = Pecos::SLOW_RESTRICTED_GROWTH;
  else // standardize rules on linear Gaussian prec: i = 2m-1 = 2(2l+1)-1 = 4l+1
    growth_rate = Pecos::MODERATE_RESTRICTED_GROWTH;

  Pecos::BasisConfigOptions bc_options(nested_rules, piecewise_basis,
				       equidist_rules, use_derivs);
  ssgDriver->initialize_grid(natafTransform.u_types(), ssgLevelRef,
    dimPrefSpec, bc_options, growth_rate, /*refine_type,*/ refine_control,
    store_colloc, track_uniq_prod_wts, track_colloc_indices);
  ssgDriver->initialize_grid_parameters(natafTransform.u_types(),
    iteratedModel.aleatory_distribution_parameters());

  maxConcurrency *= ssgDriver->grid_size(); // requires polyParams
}


/** This alternate constructor is used for on-the-fly generation and
    evaluation of sparse grids within PCE and SC. */
NonDSparseGrid::
NonDSparseGrid(Model& model, short exp_coeffs_approach,
	       const UShortArray& ssg_level_seq,
	       const RealVector& dim_pref, short growth_rate,
	       short refine_control, bool track_uniq_prod_wts,
	       bool track_colloc_indices): 
  NonDIntegration(NoDBBaseConstructor(), model, dim_pref),
  ssgLevelSeqSpec(ssg_level_seq), ssgLevelRef(ssgLevelSeqSpec[sequenceIndex])
{
  // initialize the numerical integration driver
  numIntDriver = Pecos::IntegrationDriver(exp_coeffs_approach);
  ssgDriver = (Pecos::SparseGridDriver*)numIntDriver.driver_rep();

  // propagate general settings (not inferrable from the basis of polynomials)
  // prior to initialize_grid()
  ssgDriver->growth_rate(growth_rate);
  ssgDriver->refinement_control(refine_control);
  bool store_colloc = true; //(sparse_grid_usage == Pecos::INTERPOLATION);
  ssgDriver->store_collocation_details(store_colloc);
  ssgDriver->track_unique_product_weights(track_uniq_prod_wts);
  ssgDriver->track_collocation_indices(track_colloc_indices);

  // local natafTransform not yet updated: x_types would have to be passed in
  // from NonDExpansion if check_variables() needed to be called here.  Instead,
  // it is deferred until run time in NonDIntegration::quantify_uncertainty().
  //check_variables(x_types);
}


void NonDSparseGrid::
initialize_grid(const std::vector<Pecos::BasisPolynomial>& poly_basis)
{
  ssgDriver->initialize_grid(poly_basis);
  ssgDriver->level(ssgLevelRef);
  ssgDriver->dimension_preference(dimPrefSpec);
  maxConcurrency *= ssgDriver->grid_size(); // requires polyParams
}


NonDSparseGrid::~NonDSparseGrid()
{ }


void NonDSparseGrid::get_parameter_sets(Model& model)
{
  // capture any run-time updates to distribution parameters
  if (subIteratorFlag)
    ssgDriver->initialize_grid_parameters(natafTransform.u_types(),
      iteratedModel.aleatory_distribution_parameters());

  // compute grid and retrieve point/weight sets
  ssgDriver->compute_grid(allSamples);
  Cout << "\nSparse grid level = " << ssgDriver->level() << "\nTotal number "
       << "of integration points: " << allSamples.numCols() << '\n';

  if (outputLevel > NORMAL_OUTPUT)
    print_points_weights("dakota_sparse_tabular.dat");
}


/** used by DataFitSurrModel::build_global() to publish the minimum
    number of points needed from the sparse grid routine in order to
    build a particular global approximation. */
void NonDSparseGrid::
sampling_reset(int min_samples, bool all_data_flag, bool stats_flag)
{
  // ssgLevelRef (potentially incremented from ssgLevelSeqSpec[numIntSeqIndex]
  // due to uniform/adaptive refinements) provides the current lower bound
  // reference point.  Pecos::SparseGridDriver::ssgLevel may be increased
  // ***or decreased*** to provide at least min_samples subject to this lower
  // bound.  ssgLevelRef is ***not*** updated by min_samples.

  // should be ssgLevelRef already, unless min_level previous enforced
  ssgDriver->level(ssgLevelRef);
  if (min_samples > ssgDriver->grid_size()) { // reference grid size
    // determine minimum sparse grid level that provides at least min_samples
    unsigned short min_level = ssgLevelRef + 1;
    ssgDriver->level(min_level);
    while (ssgDriver->grid_size() < min_samples)
      ssgDriver->level(++min_level);
    // leave ssgDriver at min_level; do not update ssgLevelRef

    // maxConcurrency must not be updated since parallel config management
    // depends on having the same value at ctor/run/dtor times.
  }

  // not currently used by this class:
  //allDataFlag = all_data_flag;
  //statsFlag   = stats_flag;
}


void NonDSparseGrid::increment_grid()
{
  int orig_ssg_size = ssgDriver->grid_size();
  ssgDriver->level(++ssgLevelRef);
  // with restricted growth/delayed sequences in nested rules, an increment in
  // level will not always change the grid.  Anisotropy (if present) is fixed.
  while (ssgDriver->grid_size() == orig_ssg_size)
    ssgDriver->level(++ssgLevelRef);
}


void NonDSparseGrid::increment_grid_weights(const RealVector& aniso_wts)
{
  // define reference points
  int orig_ssg_size = ssgDriver->grid_size();
  ssgDriver->update_axis_lower_bounds();
  // initial increment and anisotropy update
  ssgDriver->level(++ssgLevelRef);
  ssgDriver->anisotropic_weights(aniso_wts); // enforce axis LB's --> wt UB's
  // Enforce constraints of retaining all previous collocation sets and adding
  // at least one new set.  Given the former constraint, the same grid size
  // must logically be the same grid irregardless of changes in anisotropy.
  while (ssgDriver->grid_size() == orig_ssg_size) {
    ssgDriver->level(++ssgLevelRef);
    ssgDriver->anisotropic_weights(aniso_wts); // re-enforce LB's for new level
  }
}

} // namespace Dakota
