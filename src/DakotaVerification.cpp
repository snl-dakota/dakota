/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Verification
//- Description: Implementation code for the Verification class
//- Owner:       Mike Eldred

#include "dakota_system_defs.hpp"
#include "DakotaVerification.hpp"
#include "ProblemDescDB.hpp"
#include "dakota_data_io.hpp"

static const char rcsId[]="@(#) $Id: DakotaVerification.cpp 6886 2010-08-02 19:13:01Z mseldre $";


namespace Dakota {

Verification::Verification(ProblemDescDB& problem_db, Model& model):
  Analyzer(problem_db, model)
{
  // Check for vendor numerical gradients (manage_asv will not work properly)
  if (iteratedModel.gradient_type() == "numerical" &&
      iteratedModel.method_source() == "vendor") {
    Cerr << "\nError: Verification does not contain a vendor algorithm for "
         << "numerical derivatives;\n       please select dakota as the finite "
	 << "difference method_source." << std::endl;
    abort_handler(-1);
  }
}


Verification::Verification(unsigned short method_name, Model& model):
  Analyzer(method_name, model)
{
  // Check for vendor numerical gradients (manage_asv will not work properly)
  if (iteratedModel.gradient_type() == "numerical" &&
      iteratedModel.method_source() == "vendor") {
    Cerr << "\nError: Verification does not contain a vendor algorithm for "
         << "numerical derivatives;\n       please select dakota as the finite "
	 << "difference method_source." << std::endl;
    abort_handler(-1);
  }
}


bool Verification::resize()
{
  bool parent_reinit_comms = Analyzer::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}


void Verification::print_results(std::ostream& s)
{
  Analyzer::print_results(s);

  // TO DO
}

} // namespace Dakota
