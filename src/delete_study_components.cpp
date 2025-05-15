#include "delete_study_components.hpp"
#include "DakotaIterator.hpp"
#include "DakotaModel.hpp"

namespace Dakota {

void delete_study_components(const ProblemDescDB& problem_db) {
  Iterator::remove_cached_iterator(problem_db);
  Model::remove_cached_model(problem_db);
}

}

