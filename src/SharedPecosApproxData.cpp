/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SharedPecosApproxData
//- Description:  Implementation code for SharedPecosApproxData class
//-               
//- Owner:        Mike Eldred

#include "SharedPecosApproxData.hpp"
#include "ProblemDescDB.hpp"
#include "NonDIntegration.hpp"

//#define DEBUG


namespace Dakota {

SharedPecosApproxData::
SharedPecosApproxData(const String& approx_type,
		      const UShortArray& approx_order, size_t num_vars,
		      short data_order, short output_level):
  SharedApproxData(NoDBBaseConstructor(), approx_type, num_vars, data_order,
		   output_level)
{
  short basis_type; approx_type_to_basis_type(approxType, basis_type);

  // override selected ConfigOptions defaults, as supported by SharedApproxData
  // API.  All options are updated later in NonD*::initialize_u_space_model(),
  // so this step is not strictly required and is more for completeness.
  Pecos::ExpansionConfigOptions  ec_options; // set defaults
  Pecos::BasisConfigOptions      bc_options; // set defaults
  Pecos::RegressionConfigOptions rc_options; // set defaults
  ec_options.outputLevel = outputLevel;
  bc_options.useDerivs   = (buildDataOrder > 1);

  pecosSharedData =
    Pecos::SharedBasisApproxData(basis_type, approx_order, numVars,
				 ec_options, bc_options, rc_options);
  pecosSharedDataRep
    = (Pecos::SharedPolyApproxData*)pecosSharedData.data_rep();
}


SharedPecosApproxData::
SharedPecosApproxData(ProblemDescDB& problem_db, size_t num_vars):
  SharedApproxData(BaseConstructor(), problem_db, num_vars)
{
  short basis_type; approx_type_to_basis_type(approxType, basis_type);
  UShortArray approx_order;
  if (basis_type == Pecos::GLOBAL_ORTHOGONAL_POLYNOMIAL)
    approx_order = problem_db.get_usa("method.nond.expansion_order");

  // override selected ConfigOptions defaults, as supported by SharedApproxData
  // API.  All options are updated later in NonD*::initialize_u_space_model(),
  // so this step is not strictly required and is more for completeness.
  Pecos::ExpansionConfigOptions  ec_options; // set defaults
  Pecos::BasisConfigOptions      bc_options; // set defaults
  Pecos::RegressionConfigOptions rc_options; // set defaults
  ec_options.outputLevel = outputLevel;
  bc_options.useDerivs   = (buildDataOrder > 1);

  pecosSharedData =
    Pecos::SharedBasisApproxData(basis_type, approx_order, numVars,
				 ec_options, bc_options, rc_options);
  pecosSharedDataRep
    = (Pecos::SharedPolyApproxData*)pecosSharedData.data_rep();
}


void SharedPecosApproxData::
approx_type_to_basis_type(const String& approx_type, short& basis_type)
{
  basis_type = Pecos::NO_BASIS;
  if (strends(approx_type, "orthogonal_polynomial")) {
    if (strbegins(approx_type, "global_regression"))
      basis_type = Pecos::GLOBAL_REGRESSION_ORTHOGONAL_POLYNOMIAL;
    else if (strbegins(approx_type, "global_projection"))
      basis_type = Pecos::GLOBAL_PROJECTION_ORTHOGONAL_POLYNOMIAL;
    else if (strbegins(approx_type, "global"))
      basis_type = Pecos::GLOBAL_ORTHOGONAL_POLYNOMIAL;
    //else if (strbegins(approx_type, "piecewise_regression"))
    //  basis_type = Pecos::PIECEWISE_REGRESSION_ORTHOGONAL_POLYNOMIAL;
    //else if (strbegins(approx_type, "piecewise_projection"))
    //  basis_type = Pecos::PIECEWISE_PROJECTION_ORTHOGONAL_POLYNOMIAL;
    //else if (strbegins(approx_type, "piecewise"))
    //  basis_type = Pecos::PIECEWISE_ORTHOGONAL_POLYNOMIAL;
  }
  else if (strends(approx_type, "interpolation_polynomial")) {
    if (strbegins(approx_type, "global_nodal"))
      basis_type = Pecos::GLOBAL_NODAL_INTERPOLATION_POLYNOMIAL;
    else if (strbegins(approx_type, "global_hierarchical"))
      basis_type = Pecos::GLOBAL_HIERARCHICAL_INTERPOLATION_POLYNOMIAL;
    else if (strbegins(approx_type, "piecewise_nodal"))
      basis_type = Pecos::PIECEWISE_NODAL_INTERPOLATION_POLYNOMIAL;
    else if (strbegins(approx_type, "piecewise_hierarchical"))
      basis_type = Pecos::PIECEWISE_HIERARCHICAL_INTERPOLATION_POLYNOMIAL;
  }
}


void SharedPecosApproxData::integration_iterator(const Iterator& iterator)
{
  NonDIntegration* integration_rep = (NonDIntegration*)iterator.iterator_rep();
  pecosSharedDataRep->integration_driver_rep(
    integration_rep->driver().driver_rep());
}


void SharedPecosApproxData::link_multilevel_surrogate_data()
{
  // Manage approxDataKeys and activeDataIndex.  {surr,modSurr}Data instances
  // managed in PecosApproximation::link_multilevel_surrogate_data()

  switch (pecosSharedDataRep->discrepancy_type()) {
  case Pecos::DISTINCT_DISCREP: case Pecos::RECURSIVE_DISCREP: {
    // expand approxDataKeys from default for discrepancy management:
    approxDataKeys.resize(2); // for surrData and modSurrData
    UShort2DArray& keys_0 = approxDataKeys[0];
    UShort2DArray& keys_1 = approxDataKeys[1];
    // surrData has either HF or HF,LF keys (raw data: level 0 or levels 1-L)
    // modSurrData has HF key (combined data / discrepancy)
    keys_0.resize(2);  keys_1.resize(1);
    //keys_0[0] = keys_1[0] = pecosSharedDataRep->active_key();

    // Configure active approxData
    activeDataIndex = 0;
    // 0 for pushing raw data, 1 for pulling processed data ?
    // (other classes access the discrepancy/surplus data)
    //pushActiveDataIndex = 0;  pullActiveDataIndex = 1;
    break;
  }
  default: // default ctor linkages are sufficient
    break;
  }
}


void SharedPecosApproxData::surrogate_model_key(const UShortArray& key)
{
  // keys are organized in a 3D array: approxData instance by {truth,surrogate}
  // by multi-index key.  Note that AGGREGATED_MODELS mode uses {HF,LF} order,
  // as does ApproximationInterface::*_add()

  // Base class default implementation updates only for activeDataIndex:
  //UShort2DArray& data_keys = approxDataKeys[activeDataIndex];
  // Here, we ignore activeDataIndex and update the unmodified raw approxData
  // instance (modSurrData aggregates {HF,LF} and is keyed based on truth key).

  //size_t d, num_d = approxDataKeys.size();
  if (key.empty()) // prune second entry from each set of approxDataKeys
    /*
    for (i=0; i<num_sd; ++i)
    //if (maxNumKeys[i] > 1)      // need separate attribute to manage #keys
      if (surrogate_data_keys(i)) // since approxDataKeys shrinks/expands
        approxDataKeys[i].resize(1);
    */
    approxDataKeys[0].resize(1); // approxDataKeys[1] remains size 1
  else {
    //for (d=0; i<num_d; ++d)
    //  if (surrogate_data_keys(i)) {
    UShort2DArray& keys_0 = approxDataKeys[0];
    keys_0.resize(2);
    const UShortArray& key_00 = keys_0[0]; // HF
    UShortArray&       key_01 = keys_0[1]; // LF
    // Assign incoming LF key
    key_01 = key;
    // Alter key to distinguish a particular aggregation used for modeling
    // a discrepancy (e.g., keep lm1 distinct among l-lm1, lm1-lm2, ...) by
    // appending the HF key that matches this LF data
    key_01.insert(key_01.end(), key_00.begin(), key_00.end());
    //  }
  }
}


void SharedPecosApproxData::truth_model_key(const UShortArray& key)
{
  // keys are organized in a 3D array: approxData instance by {truth,surrogate}
  // by multi-index key.  Note that AGGREGATED_MODELS mode uses {HF,LF} order,
  // as does ApproximationInterface::*_add()

  // Base class default implementation updates only for activeDataIndex:
  //UShort2DArray& data_keys = approxDataKeys[activeDataIndex];
  // but here we will update across approxData instances (surrData,modSurrData}.

  size_t d, num_d = approxDataKeys.size();
  for (d=0; d<num_d; ++d) {
    UShort2DArray& keys_d = approxDataKeys[d];
    //if (truth_data_keys(i)) { // for completeness
    switch (keys_d.size()) { // can remain 1 if no surrogate aggregation
    case 0: keys_d.push_back(key); break;
    case 1: keys_d[0] = key;       break;
    case 2: {
      UShortArray& key_d0 = keys_d[0];
      UShortArray& key_d1 = keys_d[1];
      if (key_d0 != key) {
	// Assign HF key
	key_d0 = key;
	// Alter LF key to distinguish a particular model pair that defines a
	// discrepancy (e.g., keep lm1 distinct among l-lm1, lm1-lm2, ...)
	key_d1.resize(key_d1.size() - key.size());
	key_d1.insert(key_d1.end(), key.begin(), key.end());
      }
      break;
    }
    }
    //}
  }
}

} // namespace Dakota
