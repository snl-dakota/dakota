/*  _______________________________________________________________________ 
    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_TPLDATATRANSFER_H
#define DAKOTA_TPLDATATRANSFER_H

#include "DakotaModel.hpp"
#include "DakotaTraitsBase.hpp"

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
    void configure_data_adapters(std::shared_ptr<TraitsBase>, const Model &);

    /// Number of nonlinear equality constraints from Dakota perspective
    int num_dakota_nonlin_eq_constraints() const
      { return numDakotaNonlinearEqConstraints; }

    /// Number of nonlinear equality constraints from TPL perspective
    int num_tpl_nonlin_eq_constraints() const
      { return numTPLNonlinearEqConstraints; }

    /// Number of nonlinear inequality constraints from Dakota perspective
    int num_dakota_nonlin_ineq_constraints() const
      { return numDakotaNonlinearIneqConstraints; }

    /// Number of nonlinear inequality constraints from TPL perspective
    int num_tpl_nonlin_ineq_constraints() const
      { return numTPLNonlinearIneqConstraints; }

    //----------------------------------------------------------------

    //template <typename VecT>
    Real get_response_value_from_dakota(const Response & resp) const
    {
      return ( maxSense ? -resp.function_value(0) :
                           resp.function_value(0)   );
    }

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
      // We do not perform checks on incoming vector size or do any resizing/allocation
      // because clients may be passing a view into a larger vector, and the size/resize
      // semantics might overly constrain what type we can support, eg double*, RealVector, etc.

      if( numTPLNonlinearEqConstraints > 0 ) // TPL supports equalities
      {
        const RealVector& resp_vals = resp.function_values();

        for( size_t i=0; i<numDakotaNonlinearEqConstraints; ++i )
          values[i] =     resp_vals[numDakotaObjectiveFns + i]
                        + nonlinearEqConstraintTargets[i];
      }
    }

    //----------------------------------------------------------------

    template <typename VecT>
    void get_best_nonlinear_eq_constraints_from_tpl( const VecT & values,
                                                     RealVector & target )
    {
      // Caller is responsible for target being sized/allocated for now
      // ... we could add a check on size.
      for( size_t i=0; i<nonlinearEqConstraintMapIndices.size(); ++i )
        target[nonlinearEqConstraintMapIndices[i]] =
            values[i] - nonlinearEqConstraintTargets[i];
    }

    //----------------------------------------------------------------


  protected:

    //
    //- Heading: Granular methods for constructing various maps, etc.
    //

    /// Construct nonlinear equality maps needed to exchange data to/from Dakota and the TPL
    void configure_nonlinear_eq_adapters(NONLINEAR_EQUALITY_FORMAT, const Constraints &);

    /// Construct nonlinear inequality maps needed to exchange data to/from Dakota and the TPL
    void configure_nonlinear_ineq_adapters(NONLINEAR_INEQUALITY_FORMAT, const Constraints &, bool split_eqs);

    //
    //- Heading: Data
    //

    /// number of objective functions from Dakota perspective
    int numDakotaObjectiveFns;

    /// Single boolean (could be extended to multiple) indicating min/max sense of optimal value
    bool maxSense;

    /// number of nonlinear equality constraints from Dakota perspective
    int numDakotaNonlinearEqConstraints;

    /// number of nonlinear equality constraints from TPL perspective
    int numTPLNonlinearEqConstraints;

    /// map from Dakota constraint number to TPL constraint number
    std::vector<int> nonlinearEqConstraintMapIndices;

    /// multipliers for constraint transformations - may not be needed? - RWH
    std::vector<double> nonlinearEqConstraintMapMultipliers;

    /// offsets for constraint transformations
    std::vector<double> nonlinearEqConstraintTargets;

    /// number of nonlinear inequality constraints from Dakota perspective
    int numDakotaNonlinearIneqConstraints;

    /// number of nonlinear inequality constraints actually used
    /// ... based conditionally on lower bounds using bigRealBoundSize and on whether TPL splits equalities into two inequalities
    int numTPLNonlinearIneqConstraints;

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
