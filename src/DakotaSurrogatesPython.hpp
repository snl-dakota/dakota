/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_PYTHON_H
#define DAKOTA_SURROGATES_PYTHON_H

#include "DakotaSurrogates.hpp"


namespace Dakota {

/// Derived approximation class for Surrogates Python approximation classes.

/** This class interfaces Dakota to the Dakota Surrogates Python Module. */
class SurrogatesPythonApprox: public SurrogatesBaseApprox
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor
  SurrogatesPythonApprox() { }
  /// standard constructor for scalar surfaces: 
  SurrogatesPythonApprox(const ProblemDescDB& problem_db,
		         const SharedApproxData& shared_data,
		         const String& approx_label);
  /// standard constructor for field surfaces: 
  SurrogatesPythonApprox(const ProblemDescDB& problem_db,
		         const SharedApproxData& shared_data,
		         const StringArray& approx_labels);
  /// alternate constructor
  SurrogatesPythonApprox(const SharedApproxData& shared_data);
  /// destructor
  ~SurrogatesPythonApprox() { }

protected:

  // Minimum number of data points required to build
  int min_coefficients() const override;

  ///  Do the build - scalar version
  void build() override;

  ///  Do the build - field version
  void build(int num_resp) override;

  /// Python module filename
  String moduleFile;
};

} // namespace Dakota
#endif
