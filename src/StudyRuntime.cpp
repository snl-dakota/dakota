/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "StudyRuntime.hpp"

#include "DakotaIterator.hpp"
#include "OutputManager.hpp"
#include "ParallelLibrary.hpp"
#include "ProblemDescDB.hpp"

namespace Dakota {

StudyRuntime::StudyRuntime(ParallelLibrary* parallel_lib,
                           OutputManager* output_manager):
  parallelLib(parallel_lib),
  outputManager(output_manager)
{
}


StudyRuntime::StudyRuntime(ParallelLibrary& parallel_lib,
                           OutputManager* output_manager):
  StudyRuntime(&parallel_lib, output_manager)
{
}


void StudyRuntime::reset(ParallelLibrary& parallel_lib,
                         OutputManager* output_manager)
{
  parallelLib = &parallel_lib;
  outputManager = output_manager;
}


ParallelLibrary& StudyRuntime::parallel_library() const
{
  return *parallelLib;
}


OutputManager* StudyRuntime::output_manager() const
{
  return outputManager;
}


IteratorExecutor StudyRuntime::create_iterator_executor(
  bool peer_assign_jobs,
  int num_servers,
  int procs_per_iterator,
  short scheduling) const
{
  return IteratorExecutor(*parallelLib, peer_assign_jobs, num_servers,
                          procs_per_iterator, scheduling);
}


void StudyRuntime::initialize_top_level_iterator(
  ProblemDescDB& problem_db,
  std::shared_ptr<Iterator>& iterator,
  ParLevLIter pl_iter) const
{
  IteratorExecutor::init_iterator(problem_db, *parallelLib, iterator, pl_iter);
}


void StudyRuntime::execute_top_level_iterator(Iterator& iterator,
                                              ParLevLIter pl_iter) const
{
  execute_iterator(iterator, pl_iter);
}


void StudyRuntime::execute_iterator(Iterator& iterator,
                                    ParLevLIter pl_iter) const
{
  IteratorExecutor::run_iterator(iterator, pl_iter);
}


void StudyRuntime::free_top_level_iterator(Iterator& iterator,
                                           ParLevLIter pl_iter) const
{
  IteratorExecutor::free_iterator(iterator, pl_iter);
}


void StudyRuntime::execute_iterator(Iterator& iterator) const
{
  ParLevLIter w_pl_iter = parallelLib->w_parallel_level_iterator();
  iterator.init_communicators(w_pl_iter);
  execute_iterator(iterator, w_pl_iter);
  iterator.free_communicators(w_pl_iter);
}


void StudyRuntime::initialize_iterator(IteratorExecutor& executor,
                                       ProblemDescDB& problem_db,
                                       std::shared_ptr<Iterator>& iterator,
                                       std::shared_ptr<Model> model) const
{
  executor.init_iterator(problem_db, iterator, model);
}


void StudyRuntime::initialize_iterator(IteratorExecutor& executor,
                                       const String& method_string,
                                       std::shared_ptr<Iterator>& iterator,
                                       std::shared_ptr<Model> model) const
{
  executor.init_iterator(method_string, iterator, model);
}


void StudyRuntime::IteratorContext::initialize_child_iterator(
  std::shared_ptr<Iterator>& iterator)
{
  initialize_iterator(iterator->method_string(), iterator,
                      iterator->iterated_model());
}


void StudyRuntime::update_iterator_executor(IteratorExecutor& executor,
                                            ParConfigLIter pc_iter) const
{
  executor.update(pc_iter);
}


void StudyRuntime::update_iterator_executor(IteratorExecutor& executor,
                                            ParConfigLIter pc_iter,
                                            size_t index) const
{
  executor.update(pc_iter, index);
}


void StudyRuntime::iterator_message_lengths(IteratorExecutor& executor,
                                            int params_msg_len,
                                            int results_msg_len) const
{
  executor.iterator_message_lengths(params_msg_len, results_msg_len);
}


void StudyRuntime::set_iterator(IteratorExecutor& executor,
                                Iterator& iterator,
                                ParLevLIter pl_iter) const
{
  executor.set_iterator(iterator, pl_iter);
}


void StudyRuntime::free_iterator(IteratorExecutor& executor,
                                 Iterator& iterator) const
{
  executor.free_iterator(iterator);
}


void StudyRuntime::free_iterator_parallelism(IteratorExecutor& executor) const
{
  executor.free_iterator_parallelism();
}

} // namespace Dakota
