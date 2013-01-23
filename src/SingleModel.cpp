/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SingleModel
//- Description: Implementation code for the SingleModel class
//- Owner:       Mike Eldred
//- Checked by:

#include "dakota_system_defs.hpp"
#include "SingleModel.hpp"
#include "ProblemDescDB.hpp"

static const char rcsId[]="@(#) $Id: SingleModel.cpp 6492 2009-12-19 00:04:28Z briadam $";


using namespace std;

namespace Dakota {

// define special values for componentParallelMode
#define INTERFACE 1


SingleModel::SingleModel(ProblemDescDB& problem_db):
  Model(BaseConstructor(), problem_db),
  userDefinedInterface(problem_db.get_interface())
{
  componentParallelMode = INTERFACE;
  ignoreBounds = problem_db.get_bool("responses.ignore_bounds");
  centralHess  = problem_db.get_bool("responses.central_hess");
}


void SingleModel::component_parallel_mode(short mode)
{
  if (mode != INTERFACE) {
    Cerr << "Error: SingleModel only supports the INTERFACE component parallel "
	 << "mode." << endl;
    abort_handler(-1);
  }
  parallelLib.parallel_configuration_iterator(modelPCIter);
  //componentParallelMode = mode;
}

} // namespace Dakota
