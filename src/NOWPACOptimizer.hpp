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

  /// set {lower,upper}Bounds
  void set_unscaled_bounds(const RealVector& l_bnds, const RealVector& u_bnds);

  /// perform scaling from [lower,upper] to [0,1]
  void scale(const RealVector& unscaled_x, RealArray& scaled_x)   const;
  /// invert scaling to return from [0,1] to [lower,upper]
  void unscale(const RealArray& scaled_x, RealVector& unscaled_x) const;

private:

  /// cache a local copy of the Model
  Model iteratedModel;

  /// cache the active continuous lower bounds for scaling to [0,1]
  RealVector lowerBounds;
  /// cache the active continuous upper bounds for scaling to [0,1]
  RealVector upperBounds;

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


inline void NOWPACBlackBoxEvaluator::
set_unscaled_bounds(const RealVector& l_bnds, const RealVector& u_bnds)
{ copy_data(l_bnds, lowerBounds); copy_data(u_bnds, upperBounds); }


inline void NOWPACBlackBoxEvaluator::
scale(const RealVector& unscaled_x, RealArray& scaled_x) const
{ 
  size_t v, num_v = unscaled_x.length();
  if (scaled_x.size() != num_v)
    scaled_x.resize(num_v);
  for (v=0; v<num_v; ++v)
    scaled_x[v] = (  unscaled_x[v] - lowerBounds[v] )
                / ( upperBounds[v] - lowerBounds[v] );
}


inline void NOWPACBlackBoxEvaluator::
unscale(const RealArray& scaled_x, RealVector& unscaled_x) const
{ 
  size_t v, num_v = scaled_x.size();
  if (unscaled_x.length() != num_v)
    unscaled_x.sizeUninitialized(num_v);
  for (v=0; v<num_v; ++v)
    unscaled_x[v] = lowerBounds[v]
                  + scaled_x[v] * ( upperBounds[v] - lowerBounds[v] );
}

/**
 * \brief A version of TraitsBase specialized for NOWPAC optimizers
 *
 */

class NOWPACTraits: public TraitsBase
{
  public:

  /// default constructor
  NOWPACTraits() { }

  /// destructor
  virtual ~NOWPACTraits() { }

  /// A temporary query used in the refactor
  virtual bool is_derived() { return true; }

  /// Return the flag indicating whether method supports continuous variables
  bool supports_continuous_variables() { return true; }

  /// Return the flag indicating whether method supports linear inequalities
  bool supports_linear_inequality() { return true; }

  /// Return the flag indicating whether method supports nonlinear inequalities
  bool supports_nonlinear_inequality() { return true; }

  /// Return the format used for nonlinear inequality constraints
  NONLINEAR_INEQUALITY_FORMAT nonlinear_inequality_format()
    { return NONLINEAR_INEQUALITY_FORMAT::ONE_SIDED_LOWER; }
};


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
