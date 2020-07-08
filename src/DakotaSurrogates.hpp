/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_H
#define DAKOTA_SURROGATES_H

#include "DakotaApproximation.hpp"
#include "util_data_types.hpp"
#include <memory>


namespace dakota {
  namespace surrogates {
    class Surrogate;
  }
}

namespace Dakota {

/// Derived Approximation class for new Surrogates modules.

/** Encapsulates common behavior for Surrogates modules, with
    specialization for specific surrogates in derived classes.  */
class SurrogatesBaseApprox: public Approximation
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor
  SurrogatesBaseApprox() { }
  /// standard constructor: 
  SurrogatesBaseApprox(const ProblemDescDB& problem_db,
		       const SharedApproxData& shared_data,
		       const String& approx_label);
  /// alternate constructor
  SurrogatesBaseApprox(const SharedApproxData& shared_data);
  /// destructor
  ~SurrogatesBaseApprox() { }

  // diagnostics: all documented at base class

  bool diagnostics_available() override;

  Real diagnostic(const String& metric_type) override;

  RealArray cv_diagnostic(const StringArray& metric_types,
			  unsigned num_folds) override;

  void primary_diagnostics(int fn_index) override;

  void challenge_diagnostics(int fn_index, const RealMatrix& challenge_points,
                             const RealVector& challenge_responses) override;

  // Modify configuration options through the parameterList (non-const)
  dakota::ParameterList& getSurrogateOpts();

protected:

  /// convert Pecos surrogate data to reshaped Eigen vars/resp matrices
  void convert_surrogate_data(dakota::MatrixXd& vars, dakota::MatrixXd& resp);

  Real value(const Variables& vars) override;

  const RealVector& gradient(const Variables& vars) override;

  Real value(const RealVector& c_vars) override;

  const RealVector& gradient(const RealVector& c_vars) override;


  /// Key/value config options for underlying surrogate
  dakota::ParameterList surrogateOpts;

  /// The native surrogate model
  std::shared_ptr<dakota::surrogates::Surrogate> model;

  /// Advanced configurations options filename
  String advanced_options_file;

};

} // namespace Dakota
#endif
