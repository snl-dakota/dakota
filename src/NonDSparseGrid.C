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

#include "data_io.h"
#include "system_defs.h"
#include "NonDSparseGrid.H"
#include "DakotaModel.H"
#include "ProblemDescDB.H"

static const char rcsId[]="@(#) $Id: NonDSparseGrid.C,v 1.57 2004/06/21 19:57:32 mseldre Exp $";

//#define DEBUG

namespace Dakota {


/** This constructor is called for a standard letter-envelope iterator
    instantiation.  In this case, set_db_list_nodes has been called
    and probDescDB can be queried for settings from the method
    specification.  It is not currently used, as there is not a
    separate sparse_grid method specification. */
NonDSparseGrid::NonDSparseGrid(Model& model): NonDIntegration(model),
  ssgLevelSpec(probDescDB.get_ushort("method.nond.sparse_grid_level")),
  dimPrefSpec(
    probDescDB.get_rdv("method.nond.sparse_grid_dimension_preference")),
  ssgLevelRef(ssgLevelSpec)
{
  // initialize the numerical integration driver
  numIntDriver = Pecos::IntegrationDriver(Pecos::SPARSE_GRID);
  ssgDriver = (Pecos::SparseGridDriver*)numIntDriver.driver_rep();

  // initialize_random_variables() called in NonDIntegration ctor
  check_variables(natafTransform.x_types());
  bool store_colloc       = false; // no collocIndices/gauss{Pts,Wts}1D storage
  bool track_ensemble_wts = false;
  bool nested_rules       = true;
  bool equidistant_rules  = true;
  // moderate growth is helpful for iso and aniso sparse grids, but
  // not necessary for generalized grids
  //short refine_type
  //  = probDescDB.get_short("method.nond.expansion_refinement_type");
  short refine_control
    = probDescDB.get_short("method.nond.expansion_refinement_control");
  short growth_rate =
    (refine_control == Pecos::DIMENSION_ADAPTIVE_GENERALIZED_SPARSE) ?
    Pecos::UNRESTRICTED_GROWTH : Pecos::MODERATE_RESTRICTED_GROWTH;
  short nested_uniform_rule = Pecos::GAUSS_PATTERSON; //CLENSHAW_CURTIS,FEJER2
  ssgDriver->initialize_grid(natafTransform.u_types(), ssgLevelSpec,
    dimPrefSpec, /*refine_type,*/ refine_control, store_colloc,
    track_ensemble_wts, nested_rules, equidistant_rules, growth_rate,
    nested_uniform_rule);
  ssgDriver->initialize_grid_parameters(natafTransform.u_types(),
    iteratedModel.distribution_parameters());
  maxConcurrency *= ssgDriver->grid_size(); // requires polyParams
}


/** This alternate constructor is used for on-the-fly generation and
    evaluation of sparse grids within PCE and SC. */
NonDSparseGrid::
NonDSparseGrid(Model& model, const Pecos::ShortArray& u_types,
	       unsigned short ssg_level, const RealVector& dim_pref,
	       //short sparse_grid_usage, short refine_type,
	       short refine_control, bool track_ensemble_wts,
	       bool nested_rules): 
  NonDIntegration(NoDBBaseConstructor(), model), ssgLevelSpec(ssg_level),
  dimPrefSpec(dim_pref), ssgLevelRef(ssg_level)  
{
  // initialize the numerical integration driver
  numIntDriver = Pecos::IntegrationDriver(Pecos::SPARSE_GRID);
  ssgDriver = (Pecos::SparseGridDriver*)numIntDriver.driver_rep();

  // local natafTransform not yet updated: x_types would have to be passed in
  // from NonDExpansion if check_variables() needed to be called here.  Instead,
  // it is deferred until run time in NonDIntegration::quantify_uncertainty().
  //check_variables(x_types);
  bool  store_colloc      = true; //(sparse_grid_usage == Pecos::INTERPOLATION);
  bool  equidistant_rules = true;
  short growth_rate =
    (refine_control == Pecos::DIMENSION_ADAPTIVE_GENERALIZED_SPARSE) ?
    Pecos::UNRESTRICTED_GROWTH : Pecos::MODERATE_RESTRICTED_GROWTH;
  short nested_uniform_rule = Pecos::GAUSS_PATTERSON; //CLENSHAW_CURTIS,FEJER2
  ssgDriver->initialize_grid(u_types, ssg_level, dim_pref, //refine_type,
    refine_control, store_colloc, track_ensemble_wts, nested_rules,
    equidistant_rules, growth_rate, nested_uniform_rule);
  ssgDriver->
    initialize_grid_parameters(u_types, model.distribution_parameters());
  maxConcurrency *= ssgDriver->grid_size(); // requires polyParams
}


NonDSparseGrid::~NonDSparseGrid()
{ }


void NonDSparseGrid::get_parameter_sets(Model& model)
{
  // capture any run-time updates to distribution parameters
  if (subIteratorFlag)
    ssgDriver->initialize_grid_parameters(natafTransform.u_types(),
      iteratedModel.distribution_parameters());

  // compute grid and retrieve point/weight sets
  ssgDriver->compute_grid(allSamples);
  Cout << "\nSparse grid level = " << ssgDriver->level() << "\nTotal number "
       << "of integration points: " << allSamples.numCols() << '\n';
}


/** used by DataFitSurrModel::build_global() to publish the minimum
    number of points needed from the sparse grid routine in order to
    build a particular global approximation. */
void NonDSparseGrid::
sampling_reset(int min_samples, bool all_data_flag, bool stats_flag)
{
  // sparse grid level may be increased ***or decreased*** to provide at least
  // min_samples, but the original user specification (ssgLevelSpec) is a hard
  // lower bound.  With the introduction of uniform/adaptive refinements,
  // ssgLevelRef (which is incremented from ssgLevelSpec) replaces ssgLevelSpec
  // as the lower bound.

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
  // with "slow growth" nested rules, an increment in level will not always
  // change the grid.  Anisotropy (if present) is fixed.
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
