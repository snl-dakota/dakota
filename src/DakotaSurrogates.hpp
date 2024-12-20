/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_H
#define DAKOTA_SURROGATES_H

#include "DakotaFieldApproximation.hpp"
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
class SurrogatesBaseApprox: public FieldApproximation
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor
  SurrogatesBaseApprox() { }
  /// standard constructor for scalar surfaces: 
  SurrogatesBaseApprox(const ProblemDescDB& problem_db,
		       const SharedApproxData& shared_data,
		       const String& approx_label);
  /// standard constructor for field surfaces: 
  SurrogatesBaseApprox(const ProblemDescDB& problem_db,
		       const SharedApproxData& shared_data,
		       const StringArray& approx_labels);
  /// alternate constructor
  SurrogatesBaseApprox(const SharedApproxData& shared_data);
  /// destructor
  ~SurrogatesBaseApprox() { }

  // diagnostics: all documented at base class

  bool diagnostics_available() override;

  Real diagnostic(const String& metric_type) override;

  RealArray cv_diagnostic(const StringArray& metric_types,
			  unsigned num_folds) override;

  void primary_diagnostics(size_t fn_index) override;

  void challenge_diagnostics(size_t fn_index,
			     const RealMatrix& challenge_points,
                             const RealVector& challenge_responses) override;

  // Modify configuration options through the parameterList (non-const)
  dakota::ParameterList& getSurrogateOpts();

protected:

  /// convert Pecos surrogate data to reshaped Eigen vars/resp matrices
  void convert_surrogate_data(dakota::MatrixXd& vars, dakota::MatrixXd& resp, int num_resp=1);

  Real value(const Variables& vars) override;

  RealVector values(const Variables& vars) override;

  const RealVector& gradient(const Variables& vars) override;

  const RealSymMatrix& hessian(const Variables& vars) override;

  Real value(const RealVector& c_vars) override;

  RealVector values(const RealVector& c_vars) override;

  const RealVector& gradient(const RealVector& c_vars) override;

  const RealSymMatrix& hessian(const RealVector& c_vars) override;

  /// set the surrogate's verbosity level according to Dakota's verbosity
  void set_verbosity();

  /// construct-time only import of serialized surrogate
  void import_model(const ProblemDescDB& problem_db);

  /// validate imported labels and initialize map if needed
  void map_variable_labels(const Variables& vars);

  /// extract active or all view as vector, mapping if needed for import
  RealVector map_eval_vars(const Variables& vars);

  /// export the model to disk
  void
  export_model(const StringArray& var_labels, const String& fn_label,
	       const String& export_prefix,
	       const unsigned short export_format) override;

  void
  export_model(const Variables& vars, const String& fn_label,
	       const String& export_prefix,
	       const unsigned short export_format) override;

  /// Key/value config options for underlying surrogate
  dakota::ParameterList surrogateOpts;

  /// The native surrogate model
  std::shared_ptr<dakota::surrogates::Surrogate> model;

  /// Advanced configurations options filename
  String advanced_options_file;

  /// whether model serialized in from disk
  bool modelIsImported;
};

} // namespace Dakota
#endif
