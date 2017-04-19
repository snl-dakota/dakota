/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NOWPACOptimizer
//- Description: Wrapper class for NOWPAC
//- Owner:       Mike Eldred
//- Version: $Id: NOWPACOptimizer.hpp 6972 2010-09-17 22:18:50Z briadam $

#ifndef NOWPAC_OPTIMIZER_H
#define NOWPAC_OPTIMIZER_H

#include "DakotaOptimizer.hpp"
#include <BlackBoxBaseClass.hpp>
#include <NOWPAC.hpp>

namespace Dakota {

/// Derived class for plugging Dakota evaluations into NOWPAC solver

class NOWPACBlackBoxEvaluator: public BlackBoxBaseClass
{
public:

  /// constructor
  NOWPACBlackBoxEvaluator(Model& model);

  void evaluate(std::vector<double> const &x, // incoming params in user space
		std::vector<double> &vals, // 1 obj + len-1 nln ineq constr <= 0
		void *param); // general pass through from NOWPAC
  // alternate evaluate requires an estimate of noise in 1 obj + len-1
  // nonlin ineq constr (this is tied to SNOWPAC stochastic_optimization)
  void evaluate(std::vector<double> const &x, // incoming params in user space
		std::vector<double> &vals, // 1 obj + len-1 nln ineq constr <= 0
		std::vector<double> &noise,// 1 obj + len-1 nln ineq constr <= 0
		void *param); // general pass through from NOWPAC

  // TO DO: queue() + synchronize()

  void allocate_constraints();

  int num_ineq_constraints() const;

  const SizetList& nonlinear_inequality_mapping_indices()     const;
  const RealList&  nonlinear_inequality_mapping_multipliers() const;
  const RealList&  nonlinear_inequality_mapping_offsets()     const;

private:

  /// cache a local copy of the Model
  Model iteratedModel;

  /// aggregate unsupported constraint types as nonlinear inequalities
  int numNowpacIneqConstr;

  /// a list of indices for referencing the DAKOTA nonlinear inequality
  /// constraints used in computing the corresponding NOWPAC constraints.
  SizetList nonlinIneqConMappingIndices;
  /// a list of multipliers for mapping the DAKOTA nonlinear inequality
  /// constraints to the corresponding NOWPAC constraints.
  RealList nonlinIneqConMappingMultipliers;
  /// a list of offsets for mapping the DAKOTA nonlinear inequality
  /// constraints to the corresponding NOWPAC constraints.
  RealList nonlinIneqConMappingOffsets;

  /// a list of indices for referencing the DAKOTA linear inequality
  /// constraints used in computing the corresponding NOWPAC constraints.
  SizetList linIneqConMappingIndices;
  /// a list of multipliers for mapping the DAKOTA linear inequality
  /// constraints to the corresponding NOWPAC constraints.
  RealList linIneqConMappingMultipliers;
  /// a list of offsets for mapping the DAKOTA linear inequality
  /// constraints to the corresponding NOWPAC constraints.
  RealList linIneqConMappingOffsets;
};


inline NOWPACBlackBoxEvaluator::NOWPACBlackBoxEvaluator(Model& model):
  iteratedModel(model)
{ }


inline int NOWPACBlackBoxEvaluator::num_ineq_constraints() const
{ return numNowpacIneqConstr; }


inline const SizetList& NOWPACBlackBoxEvaluator::
nonlinear_inequality_mapping_indices() const
{ return nonlinIneqConMappingIndices; }


inline const RealList& NOWPACBlackBoxEvaluator::
nonlinear_inequality_mapping_multipliers() const
{ return nonlinIneqConMappingMultipliers; }


inline const RealList& NOWPACBlackBoxEvaluator::
nonlinear_inequality_mapping_offsets() const
{ return nonlinIneqConMappingOffsets; }


/// Wrapper class for the (S)NOWPAC optimization algorithms from
/// Florian Augustin (MIT)

class NOWPACOptimizer: public Optimizer
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NOWPACOptimizer(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor
  NOWPACOptimizer(Model& model);
  /// destructor
  ~NOWPACOptimizer();

  //
  //- Heading: Virtual member function redefinitions
  //

  void core_run();

protected:

  //
  //- Heading: Virtual member function redefinitions
  //

  // performs run-time set up
  //void initialize_run();

private:

  //
  //- Heading: Convenience member functions
  //

  void initialize_options();   ///< Shared constructor code

  //
  //- Heading: Data
  //

  NOWPAC<> nowpacSolver;

  NOWPACBlackBoxEvaluator nowpacEvaluator;
};


#ifdef HAVE_DYNLIB_FACTORIES
// ---------------------------------------------------------
// Factory functions for dynamic loading of solver libraries
// ---------------------------------------------------------

NOWPACOptimizer* new_NOWPACOptimizer(ProblemDescDB& problem_db, Model& model);
NOWPACOptimizer* new_NOWPACOptimizer(Model& model);
#endif // HAVE_DYNLIB_FACTORIES

} // namespace Dakota

#endif // NOWPAC_OPTIMIZER_H
