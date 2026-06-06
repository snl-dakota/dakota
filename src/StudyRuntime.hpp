#ifndef STUDY_RUNTIME_H
#define STUDY_RUNTIME_H

#include "IteratorExecutor.hpp"

namespace Dakota {

class OutputManager;

/// Shared execution runtime for a constructed Dakota study graph.
class StudyRuntime
{
public:
  StudyRuntime(ParallelLibrary* parallel_lib = nullptr,
               OutputManager* output_manager = nullptr);

  StudyRuntime(ParallelLibrary& parallel_lib,
               OutputManager* output_manager = nullptr);

  void reset(ParallelLibrary& parallel_lib,
             OutputManager* output_manager = nullptr);

  ParallelLibrary& parallel_library() const;
  OutputManager* output_manager() const;

  IteratorExecutor create_iterator_executor(
    bool peer_assign_jobs,
    int num_servers = 0,
    int procs_per_iterator = 0,
    short scheduling = DEFAULT_SCHEDULING) const;

  void initialize_top_level_iterator(ProblemDescDB& problem_db,
                                     std::shared_ptr<Iterator>& iterator,
                                     ParLevLIter pl_iter) const;

  void execute_top_level_iterator(Iterator& iterator,
                                  ParLevLIter pl_iter) const;

  void execute_iterator(Iterator& iterator,
                        ParLevLIter pl_iter) const;

  void free_top_level_iterator(Iterator& iterator,
                               ParLevLIter pl_iter) const;

  void execute_iterator(Iterator& iterator) const;

  void initialize_iterator(IteratorExecutor& executor,
                           ProblemDescDB& problem_db,
                           std::shared_ptr<Iterator>& iterator,
                           std::shared_ptr<Model> model) const;

  void initialize_iterator(IteratorExecutor& executor,
                           const String& method_string,
                           std::shared_ptr<Iterator>& iterator,
                           std::shared_ptr<Model> model) const;

  void update_iterator_executor(IteratorExecutor& executor,
                              ParConfigLIter pc_iter) const;

  void update_iterator_executor(IteratorExecutor& executor,
                              ParConfigLIter pc_iter,
                              size_t index) const;

  void iterator_message_lengths(IteratorExecutor& executor,
                                int params_msg_len,
                                int results_msg_len) const;

  void set_iterator(IteratorExecutor& executor,
                    Iterator& iterator,
                    ParLevLIter pl_iter) const;

  void free_iterator(IteratorExecutor& executor,
                     Iterator& iterator) const;

  void free_iterator_parallelism(IteratorExecutor& executor) const;

  template <typename MetaType>
  void schedule_iterators(IteratorExecutor& executor,
                          MetaType& meta_object,
                          Iterator& iterator) const
  {
    executor.schedule_iterators(meta_object, iterator);
  }

private:
  ParallelLibrary* parallelLib;
  OutputManager* outputManager;
};

} // namespace Dakota

#endif
