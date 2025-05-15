#include "delete_study_components.hpp"
#include "DakotaIterator.hpp"
#include "DakotaModel.hpp"
#include "DakotaInterface.hpp"

namespace Dakota {

void delete_study_components(const ProblemDescDB& problem_db) {
  Iterator::remove_cached_iterator(problem_db);
  Model::remove_cached_model(problem_db);
  Interface::remove_cached_interface(problem_db);
}

}

