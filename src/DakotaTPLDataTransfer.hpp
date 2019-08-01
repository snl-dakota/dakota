/*  _______________________________________________________________________ 
    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Optimizer
//- Description:  A class to facilitate data transfers between Dakota and a TPL
//- Owner:        Russell Hooper

#ifndef DAKOTA_TPLDATATRANSFER_H
#define DAKOTA_TPLDATATRANSFER_H

// Might replace this with DakotaModel.hpp and add others as needed - RWH
#include "DakotaMinimizer.hpp"

namespace Dakota {

//----------------------------------------------------------------


/** The TPLDataTransfer class provides ...
 */

class TPLDataTransfer
{
  public:

    /// default constructor
    TPLDataTransfer();

    /// destructor
    ~TPLDataTransfer() { }

    /// Construct maps, etc. needed to exchange data to/from Dakota and the TPL
    void configure_data_adapters(std::shared_ptr<TraitsBase>, const Constraints &);

    /// Number of nonlinear equality constraints
    int num_nonlin_eq_constraints() const
      { return numDakotaNonlinearEqConstraints; }

    /// Number of active nonlinear inequality constraints
    int num_active_nonlin_ineq_constraints() const
      { return numNonlinearIneqConstraintsActive; }

    //----------------------------------------------------------------

    template <typename VecT>
    void get_nonlinear_ineq_constraints_from_dakota( const Response & resp,
                                                               VecT & values )
    {
      const RealVector& resp_vals = resp.function_values();

      for( size_t i=0; i<nonlinearIneqConstraintMapIndices.size(); ++i )
        values[i] =   resp_vals[nonlinearIneqConstraintMapIndices[i]]
                      * nonlinearIneqConstraintMapMultipliers[i]
                      + nonlinearIneqConstraintMapShifts[i];
    }

    //----------------------------------------------------------------

    template <typename VecT>
    void get_best_nonlinear_ineq_constraints_from_tpl( const VecT & values,
                                                       RealVector & target )
    {
      // Caller is responsible for target being sized/allocated for now
      // ... we could add a check on size.
      for( size_t i=0; i<nonlinearIneqConstraintMapIndices.size(); ++i )
        target[nonlinearIneqConstraintMapIndices[i]] =
            (values[i] - nonlinearIneqConstraintMapShifts[i]) /
                         nonlinearIneqConstraintMapMultipliers[i];
    }

    //----------------------------------------------------------------

    template <typename VecT>
    void get_nonlinear_eq_constraints_from_dakota( const Response & resp,
                                                             VecT & values )
    {
      const RealVector& resp_vals = resp.function_values();

      for( size_t i=0; i<numDakotaNonlinearEqConstraints; ++i )
        values[i] =     resp_vals[numDakotaObjectiveFns + i]
                      + nonlinearEqConstraintTargets[i];
    }

    //----------------------------------------------------------------


  protected:

    //
    //- Heading: Granular methods for constructing various maps, etc.
    //

    /// Construct nonlinear equality maps needed to exchange data to/from Dakota and the TPL
    void configure_nonlinear_eq_adapters(NONLINEAR_EQUALITY_FORMAT, const Constraints &);

    /// Construct nonlinear inequality maps needed to exchange data to/from Dakota and the TPL
    void configure_nonlinear_ineq_adapters(NONLINEAR_INEQUALITY_FORMAT, const Constraints &);

    //
    //- Heading: Data
    //

    /// number of objective functions from Dakota perspective
    int numDakotaObjectiveFns;

    /// number of nonlinear equality constraints from Dakota perspective
    int numDakotaNonlinearEqConstraints;

    /// map from Dakota constraint number to TPL constraint number
    std::vector<int> nonlinearEqConstraintMapIndices;

    /// multipliers for constraint transformations - may not be needed? - RWH
    std::vector<double> nonlinearEqConstraintMapMultipliers;

    /// offsets for constraint transformations
    std::vector<double> nonlinearEqConstraintTargets;

    /// number of nonlinear inequality constraints from Dakota perspective
    int numDakotaNonlinearIneqConstraints;

    /// number of nonlinear inequality constraints actually used (based on conditional and bigRealBoundSize
    int numNonlinearIneqConstraintsActive;

    /// map from Dakota constraint number to TPL constraint number
    std::vector<int> nonlinearIneqConstraintMapIndices;

    /// multipliers for constraint transformations
    std::vector<double> nonlinearIneqConstraintMapMultipliers;

    /// offsets for constraint transformations
    std::vector<double> nonlinearIneqConstraintMapShifts;


  private:

};

} // namespace Dakota

#endif
