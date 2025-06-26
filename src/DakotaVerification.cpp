/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaVerification.hpp"

#include "ProblemDescDB.hpp"
#include "dakota_data_io.hpp"
#include "dakota_system_defs.hpp"

static const char rcsId[] =
    "@(#) $Id: DakotaVerification.cpp 6886 2010-08-02 19:13:01Z mseldre $";

namespace Dakota {

Verification::Verification(ProblemDescDB& problem_db,
                           ParallelLibrary& parallel_lib,
                           std::shared_ptr<Model> model)
    : Analyzer(problem_db, parallel_lib, model) {
  // Check for vendor numerical gradients (manage_asv will not work properly)
  if (iteratedModel->gradient_type() == "numerical" &&
      iteratedModel->method_source() == "vendor") {
    Cerr << "\nError: Verification does not contain a vendor algorithm for "
         << "numerical derivatives;\n       please select dakota as the finite "
         << "difference method_source." << std::endl;
    abort_handler(-1);
  }
}

Verification::Verification(unsigned short method_name,
                           std::shared_ptr<Model> model)
    : Analyzer(method_name, model) {
  // Check for vendor numerical gradients (manage_asv will not work properly)
  if (iteratedModel->gradient_type() == "numerical" &&
      iteratedModel->method_source() == "vendor") {
    Cerr << "\nError: Verification does not contain a vendor algorithm for "
         << "numerical derivatives;\n       please select dakota as the finite "
         << "difference method_source." << std::endl;
    abort_handler(-1);
  }
}

bool Verification::resize() {
  bool parent_reinit_comms = Analyzer::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}

void Verification::print_results(std::ostream& s, short results_state) {
  Analyzer::print_results(s, results_state);

  // TO DO
}

}  // namespace Dakota
