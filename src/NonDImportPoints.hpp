/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_IMPORT_POINTS_H
#define NOND_IMPORT_POINTS_H

#include "NonDSampling.hpp"
#include "DataMethod.hpp"

namespace Dakota {

/// Imports points for processing in Dakota. 

/** The import points capability imports input-output points
    and computes statistics on the points.
*/
class NonDImportPoints: public NonDSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDImportPoints(ProblemDescDB& problem_db, ParallelLibrary& parallel_lib,  std::shared_ptr<Model> model);
  ~NonDImportPoints() override;

  //
  //- Heading: Virtual function redefinitions
  //

  /// Import points from file. 
  void pre_run() override;

  /// Override NonDSampling's core_run so there's a no-op.
  void core_run() override;
  
  /// Compute statistics for imported points
  void post_run(std::ostream& s) override;

  /// Read input-output points from file.
  void import_points();

  /// print the final statistics
  void print_results(std::ostream& s, short results_state = FINAL_RESULTS) override;

  //
  //- Heading: Member functions
  //

private:

  //
  //- Heading: Data
  //

  /// number of response functions; used to distinguish NonD from opt/NLS usage
  size_t numResponseFunctions;

  /// sampling method for computing variance-based decomposition indices
  unsigned short vbdViaSamplingMethod;

  /// number of bins for using with the Mahadevan sampling method for computing variance-based decomposition indices
  int vbdViaSamplingNumBins;

  /// name of file containing points to import
  String importPointsFile;

  /// Format of the import points file
  unsigned short importPointsFormat;

  bool importUseVariableLabels;
  bool importActiveVariablesOnly;

};

} // namespace Dakota

#endif
