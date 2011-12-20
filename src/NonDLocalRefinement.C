/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDLocalRefinement
//- Description: Implementation code for NonDLocalRefinement class
//- Owner:       Mike Eldred
//- Revised by:  
//- Version:

#include "data_types.h"
#include "system_defs.h"
#include "NonDLocalRefinement.H"
#include "DakotaModel.H"
#include "ProblemDescDB.H"
#include "PolynomialApproximation.hpp"

static const char rcsId[]="@(#) $Id: NonDLocalRefinement.C,v 1.57 2004/06/21 19:57:32 mseldre Exp $";


namespace Dakota {

/** This alternate constructor is used for on-the-fly generation and
    evaluation of numerical integration points. */
NonDLocalRefinement::
NonDLocalRefinement(Model& model, const UShortArray& lr_level,
		    const RealVector& dim_pref):
  NonDIntegration(NoDBBaseConstructor(), model, dim_pref), lrLevelSpec(lr_level)
{
  // initialize the numerical integration driver
  numIntDriver = Pecos::IntegrationDriver(Pecos::LOCAL_REFINABLE);
  lrDriver = (Pecos::LocalRefinableDriver*)numIntDriver.driver_rep();

  // local natafTransform not yet updated: x_types would have to be passed in
  // from NonDExpansion if check_variables() needed to be called here.  Instead,
  // it is deferred until run time in NonDIntegration::quantify_uncertainty().
  //check_variables(x_types);
}


NonDLocalRefinement::~NonDLocalRefinement()
{ }


/** Used in combination with alternate NonDLocalRefinement constructor. */
void NonDLocalRefinement::
initialize_grid(const std::vector<Pecos::BasisPolynomial>& poly_basis)
{
  short basis_type0 = poly_basis[0].basis_type();
  for (size_t i=1; i<numContinuousVars; ++i)
    if (poly_basis[i].basis_type() != basis_type0) {
      Cerr << "Error: basis types must be isotropic in NonDLocalRefinement::"
	   << "initialize_grid(poly_basis)." << std::endl;
      abort_handler(-1);
    }
  bool use_derivs = (basis_type0 == Pecos::PIECEWISE_CUBIC_INTERP);
  RealArray lower_bnds, upper_bnds;
  copy_data(iteratedModel.continuous_lower_bounds(), lower_bnds);
  copy_data(iteratedModel.continuous_upper_bounds(), upper_bnds);
  lrDriver->initialize_grid(lower_bnds, upper_bnds, lrLevelSpec[sequenceIndex],
			    basis_type0, use_derivs);
  //reset();
  maxConcurrency *= lrDriver->grid_size();
}


void NonDLocalRefinement::get_parameter_sets(Model& model)
{
  // capture any distribution parameter insertions
  if (!numIntegrations || subIteratorFlag)
    lrDriver->initialize_grid_parameters(natafTransform.u_types(),
      iteratedModel.distribution_parameters());

  // Compute the grid and store in allSamples
  Cout << "\nTotal number of integration points: " << lrDriver->grid_size()
       << '\n';
  lrDriver->compute_grid(allSamples);
  if (outputLevel > NORMAL_OUTPUT)
    print_points_weights("dakota_local_refine_tabular.dat");
}


/** used by DataFitSurrModel::build_global() to publish the minimum
    number of points needed to build a particular global approximation. */
void NonDLocalRefinement::
sampling_reset(int min_samples, bool all_data_flag, bool stats_flag)
{
  if (min_samples > lrDriver->grid_size()) // update lrDriver
    lrDriver->refine_globally();

  // not currently used by this class:
  //allDataFlag = all_data_flag;
  //statsFlag   = stats_flag;
}

} // namespace Dakota
