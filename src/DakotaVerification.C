/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Verification
//- Description: Implementation code for the Verification class
//- Owner:       Mike Eldred

#include "system_defs.h"
#include "DakotaVerification.H"
#include "ProblemDescDB.H"
#include "data_io.h"

static const char rcsId[]="@(#) $Id: DakotaVerification.C 6886 2010-08-02 19:13:01Z mseldre $";


namespace Dakota {

Verification::Verification(Model& model): Analyzer(model)
{
  // Check for vendor numerical gradients (manage_asv will not work properly)
  if (gradientType == "numerical" && methodSource == "vendor") {
    Cerr << "\nError: Verification does not contain a vendor algorithm for "
         << "numerical derivatives;\n       please select dakota as the finite "
	 << "difference method_source." << std::endl;
    abort_handler(-1);
  }
}


Verification::Verification(NoDBBaseConstructor, Model& model):
  Analyzer(NoDBBaseConstructor(), model)
{
  // Check for vendor numerical gradients (manage_asv will not work properly)
  if (gradientType == "numerical" && methodSource == "vendor") {
    Cerr << "\nError: Verification does not contain a vendor algorithm for "
         << "numerical derivatives;\n       please select dakota as the finite "
	 << "difference method_source." << std::endl;
    abort_handler(-1);
  }
}


void Verification::print_results(std::ostream& s)
{
  Analyzer::print_results(s);

  // TO DO
}

} // namespace Dakota
