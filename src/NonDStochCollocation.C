/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDStochCollocation
//- Description: Implementation code for NonDStochCollocation class
//- Owner:       Mike Eldred

#include "system_defs.h"
#include "NonDStochCollocation.H"
#include "DakotaModel.H"
#include "DakotaResponse.H"
#include "ProblemDescDB.H"
#include "DataFitSurrModel.H"


namespace Dakota {

NonDStochCollocation::NonDStochCollocation(Model& model): NonDExpansion(model)
{
  // This constructor is called for a standard letter-envelope iterator 
  // instantiation.

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  construct_g_u_model(g_u_model);

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  // LHS/Incremental LHS/Quadrature/SparseGrid samples in u-space
  // generated using active sampling view:
  Iterator u_space_sampler;
  const UShortArray& quad_order_spec
    = probDescDB.get_dusa("method.nond.quadrature_order");
  unsigned short ssg_level_spec
    = probDescDB.get_ushort("method.nond.sparse_grid_level");
  if (!quad_order_spec.empty())
    construct_quadrature(u_space_sampler, g_u_model, quad_order_spec);
  else if (ssg_level_spec != USHRT_MAX)
    construct_sparse_grid(u_space_sampler, g_u_model, ssg_level_spec,
      probDescDB.get_rdv("method.nond.sparse_grid_dimension_preference"));
  // iteratedModel concurrency is defined by the number of samples
  // used in constructing the expansion
  if (numSamplesOnModel) // optional with default = 0
    maxConcurrency *= numSamplesOnModel;

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for PCBDO with polynomials over {u}+{d}, change view to All.
  String approx_type = "global_interpolation_polynomial", sample_reuse,
    corr_type;
  UShortArray approx_order; // empty
  short corr_order = -1;
  //const Variables& g_u_vars = g_u_model.current_variables();
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
    //g_u_vars.view(), g_u_vars.variables_components(),
    //g_u_model.current_response().active_set(),
    approx_type, approx_order, corr_type, corr_order, sample_reuse), false);
  initialize_u_space_model();

  // -------------------------------
  // Construct expSampler, if needed
  // -------------------------------
  construct_expansion_sampler();

  // uSpaceModel concurrency is defined by the number of samples used
  // in evaluating the stochastic expansion
  uSpaceModel.init_communicators(
    numSamplesOnExpansion*uSpaceModel.derivative_concurrency());
}


NonDStochCollocation::~NonDStochCollocation()
{
  uSpaceModel.free_communicators(
    numSamplesOnExpansion*uSpaceModel.derivative_concurrency());
}

} // namespace Dakota
