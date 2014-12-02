/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaUtils.hpp"
#include "DartSerialDirectApplicInterface.hpp"
#include "PRPMultiIndex.hpp"

using namespace Dakota;

void DART::connect_plugin(Dakota::ProblemDescDB *problem_db, DART::DakotaFunctor *f) {
  ModelList& models = problem_db->model_list();
  for (ModelLIter ml_iter = models.begin(); ml_iter != models.end(); ml_iter++){
    Interface& interface = ml_iter->derived_interface();
    if ((interface.interface_type() & DIRECT_INTERFACE_BIT) &&
	 contains(interface.analysis_drivers(), "plugin_dart") ) {
      // set the DB nodes to that of the existing Model specification
      problem_db->set_db_model_nodes(ml_iter->model_id());
      // plug in the new derived Interface object
      interface.assign_rep(new DartSerialDirectApplicInterface(*problem_db, f), false);
    }
  }
}

namespace Dakota {

  extern PRPCache data_pairs;
};

void DART::clear_prp_cache() {
  data_pairs.clear();
}
