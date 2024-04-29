/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#ifndef SURFPACK_APPROXIMATION_H 
#define SURFPACK_APPROXIMATION_H 

#include "DakotaApproximation.hpp"


// Pertinent classes from Surfpack
class SurfData;
class SurfpackModel;
class SurfpackModelFactory;
template< typename T > class SurfpackMatrix;

namespace Dakota {

class SharedApproxData;


/// Derived approximation class for Surfpack approximation classes.
/// Interface between Surfpack and Dakota.

/** The SurfpackApproximation class is the interface between Dakota
    and Surfpack.  Based on the information in the ProblemDescDB that
    is passed in through the constructor, SurfpackApproximation builds
    a Surfpack Surface object that corresponds to one of the following
    data-fitting techniques: polynomial regression, kriging,
    artificial neural networks, radial basis function network, or
    multivariate adaptaive regression splines (MARS). */

class SurfpackApproximation: public Approximation
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor
  SurfpackApproximation();
  /// standard constructor: Surfpack surface of appropriate type will be created
  SurfpackApproximation(const ProblemDescDB& problem_db,
			const SharedApproxData& shared_data,
                        const String& approx_label);
  /// alternate constructor
  SurfpackApproximation(const SharedApproxData& shared_data);
  /// destructor
  ~SurfpackApproximation() { }

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  int min_coefficients() const override;
  int recommended_coefficients() const override;
  //int num_constraints() const; // use default implementation

  /// SurfData object will be created from Dakota's SurrogateData,
  /// and the appropriate Surfpack build method will be invoked
  void build() override;


  /// validate imported labels and initialize map if needed
  void map_variable_labels(const Variables& vars);

  /// export the Surpack model to disk or console
  void export_model(const StringArray& var_labels, const String& fn_label,
		    const String& export_prefix,
                    const unsigned short export_format) override;

  void
  export_model(const Variables& vars, const String& fn_label,
	       const String& export_prefix,
	       const unsigned short export_format) override;


  // return #coefficients
  //RealVector approximation_coefficients(bool normalized) const;
  // set #coefficients
  //void approximation_coefficients(const RealVector& approx_coeffs,
  //                                bool normalized);

  /// Return the value of the Surfpack surface for a given parameter vector x
  Real value(const Variables& vars) override;
  /// retrieve the approximate function gradient for a given parameter vector x
  const RealVector& gradient(const Variables& vars) override;
  /// retrieve the approximate function Hessian for a given parameter vector x
  const RealSymMatrix& hessian(const Variables& vars) override;
  /// retrieve the variance of the predicted value for a given parameter set x
  /// (KrigingModel only)
  Real prediction_variance(const Variables& vars) override;
    
  /// Return the value of the Surfpack surface for a given parameter vector x
  Real value(const RealVector& c_vars) override;
  /// retrieve the approximate function gradient for a given parameter vector x
  const RealVector& gradient(const RealVector& c_vars) override;
  /// retrieve the approximate function Hessian for a given parameter vector x
  const RealSymMatrix& hessian(const RealVector& c_vars) override;
  /// retrieve the variance of the predicted value for a given parameter set x
  /// (KrigingModel only)
  Real prediction_variance(const RealVector& c_vars) override;

  /// check if the diagnostics are available (true for the Surfpack types)
  bool diagnostics_available() override;
  /// retrieve a single diagnostic metric for the diagnostic type specified
  /// on the primary model and data
  Real diagnostic(const String& metric_type) override;
  /// retrieve a single diagnostic metric for the diagnostic type specified
  /// on the given model and data - not inherited
  Real diagnostic(const String& metric_type, const SurfpackModel& model,
		  const SurfData& data);

  /// compute and print all requested diagnostics and cross-validation 
  void primary_diagnostics(size_t fn_index) override;
  /// compute and print all requested diagnostics for user provided
  /// challenge pts
  void challenge_diagnostics(size_t fn_index,
			     const RealMatrix& challenge_points,
                             const RealVector& challenge_responses) override;

  /// compute and return cross-validation for metric_type with num_folds
  RealArray cv_diagnostic(const StringArray& metric_types, unsigned num_folds) override;
  
  /// compute and print all requested diagnostics for user provided
  /// challenge pts
  RealArray challenge_diagnostic(const StringArray& metric_types,
			    const RealMatrix& challenge_points,
                            const RealVector& challenge_responses) override;
 
private:

  //
  //- Heading: Convenience functions
  //

  /// construct-time only import of serialized surrogate
  void import_model(const ProblemDescDB& problem_db);

  /// copy from SurrogateData to SurfPoint/SurfData in surfData
  void surrogates_to_surf_data();

  /// set the anchor point (including gradient and hessian if present)
  /// into surf_data
  void add_constraints_to_surfdata(const Pecos::SurrogateDataVars& anchor_vars,
				   const Pecos::SurrogateDataResp& anchor_resp,
				   short fail_code);

  /// extract active or all view as vector, mapping if needed for import
  RealArray map_eval_vars(const Variables& vars);

  //
  //- Heading: Data
  //

  // Vector representation of the Approximation (e.g., polynomial coefficients
  // for linear regression or trained neural network weights).  The format of
  // such a vector has not been defined for all Surfpack classes
  //RealVector coefficients;

  /// The native Surfpack approximation
  std::shared_ptr<SurfpackModel> spModel;
  /// factory for the SurfpackModel instance
  std::shared_ptr<SurfpackModelFactory> spFactory;
  /// The data used to build the approximation, in Surfpack format
  std::shared_ptr<SurfData> surfData;

  // convenience pointer to shared data representation
  //std::shared_ptr<SharedSurfpackApproxData> sharedSurfDataRep;

  /// whether model serialized in from disk
  bool modelIsImported;
};


inline SurfpackApproximation::SurfpackApproximation() // : sharedDataRep(NULL)
{ }


inline bool SurfpackApproximation::diagnostics_available()
{ return true; }


/* The value returned from this function is currently meaningless.
   \todo: Provide an appropriate list of coefficients for each surface type */
//inline RealVector SurfpackApproximation::
//approximation_coefficients(bool normalized) const
//{ return coefficients; }


//inline void SurfpackApproximation::
//approximation_coefficients(const RealVector& approx_coeffs, bool normalized)
//{ coefficients = approx_coeffs; }

} // namespace Dakota
#endif
