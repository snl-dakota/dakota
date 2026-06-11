#ifndef STUDY_RUNTIME_H
#define STUDY_RUNTIME_H

#include "IteratorExecutor.hpp"

namespace Dakota {

class OutputManager;

/// Shared execution runtime for a constructed Dakota study graph.
class StudyRuntime
{
public:
  class IteratorContext
  {
  public:
    IteratorContext(const StudyRuntime& runtime_in,
                    bool peer_assign_jobs,
                    int num_servers = 0,
                    int procs_per_iterator = 0,
                    short scheduling = DEFAULT_SCHEDULING):
      parallelLib(&runtime_in.parallel_library()),
      outputManager(runtime_in.output_manager()),
      executor(runtime_in.parallel_library(), peer_assign_jobs, num_servers,
               procs_per_iterator, scheduling)
    {
    }

    void construct_sub_iterator(ProblemDescDB& problem_db,
                                ParallelLibrary& parallel_lib,
                                std::shared_ptr<Iterator>& sub_iterator,
                                std::shared_ptr<Model> sub_model,
                                const String& method_ptr,
                                const String& method_name,
                                const String& model_ptr)
    {
      executor.construct_sub_iterator(problem_db, parallel_lib, sub_iterator,
                                      sub_model, method_ptr, method_name,
                                      model_ptr);
    }
    void reset_configuration(bool peer_assign_jobs,
                             int num_servers = 0,
                             int procs_per_iterator = 0,
                             short scheduling = DEFAULT_SCHEDULING)
    {
      executor.peerAssignJobs = peer_assign_jobs;
      executor.numIteratorServers = num_servers;
      executor.procsPerIterator = procs_per_iterator;
      executor.iteratorScheduling = scheduling;
    }


    IntIntPair configure(ProblemDescDB& problem_db,
                         std::shared_ptr<Iterator>& sub_iterator,
                         std::shared_ptr<Model> sub_model)
    {
      return executor.configure(problem_db, sub_iterator, sub_model);
    }

    IntIntPair configure(ProblemDescDB& problem_db,
                         const String& method_string,
                         std::shared_ptr<Iterator>& sub_iterator,
                         std::shared_ptr<Model> sub_model)
    {
      return executor.configure(problem_db, method_string, sub_iterator,
                                sub_model);
    }

    IntIntPair configure(ProblemDescDB& problem_db,
                         std::shared_ptr<Iterator>& sub_iterator)
    {
      return executor.configure(problem_db, sub_iterator);
    }
    IntIntPair configure(std::shared_ptr<Iterator>& sub_iterator)
    {
      return executor.configure(sub_iterator);
    }

    void partition(int max_iterator_concurrency, IntIntPair& ppi_pr)
    {
      executor.partition(max_iterator_concurrency, ppi_pr);
    }

    void initialize_iterator(ProblemDescDB& problem_db,
                             std::shared_ptr<Iterator>& iterator,
                             std::shared_ptr<Model> model)
    {
      runtime().initialize_iterator(executor, problem_db, iterator, model);
    }

    void initialize_iterator(const String& method_string,
                             std::shared_ptr<Iterator>& iterator,
                             std::shared_ptr<Model> model)
    {
      runtime().initialize_iterator(executor, method_string, iterator, model);
    }

    void update(ParConfigLIter pc_iter)
    {
      runtime().update_iterator_executor(executor, pc_iter);
    }

    void update(ParConfigLIter pc_iter, size_t index)
    {
      runtime().update_iterator_executor(executor, pc_iter, index);
    }

    void iterator_message_lengths(int params_msg_len, int results_msg_len)
    {
      runtime().iterator_message_lengths(executor, params_msg_len,
                                       results_msg_len);
    }

    void set_iterator(Iterator& iterator, ParLevLIter pl_iter)
    {
      runtime().set_iterator(executor, iterator, pl_iter);
    }

    void execute_iterator(Iterator& iterator, ParLevLIter pl_iter) const
    {
      runtime().execute_iterator(iterator, pl_iter);
    }

    void free_iterator(Iterator& iterator)
    {
      runtime().free_iterator(executor, iterator);
    }

    void free_iterator_parallelism()
    {
      runtime().free_iterator_parallelism(executor);
    }

    template <typename MetaType>
    void schedule_iterators(MetaType& meta_object, Iterator& iterator)
    {
      runtime().schedule_iterators(executor, meta_object, iterator);
    }

    void stop_iterator_servers()
    {
      executor.stop_iterator_servers();
    }

    bool lead_rank() const
    {
      return executor.lead_rank();
    }

    bool active_server() const
    {
      return iteratorServerId() <= numIteratorServers();
    }

    bool idle_partition() const
    {
      return !active_server();
    }

    bool iterator_comm_lead() const
    {
      return iteratorCommRank() == 0;
    }

    bool graphics_server() const
    {
      int server_id = iteratorServerId();
      return iterator_comm_lead() && server_id > 0 &&
             server_id <= numIteratorServers();
    }

    bool scheduler_rank() const
    {
      return iteratorServerId() == 0;
    }

    bool dedicated_scheduler() const
    {
      return iteratorScheduling() == DEDICATED_SCHEDULER_DYNAMIC;
    }

    bool peer_scheduling() const
    {
      return iteratorScheduling() == PEER_SCHEDULING;
    }

    bool multiple_iterator_servers() const
    {
      return numIteratorServers() > 1;
    }

    bool iterator_comm_parallel() const
    {
      return iteratorCommSize() > 1;
    }

    bool peer_assigns_local_jobs() const
    {
      return peer_scheduling() && peerAssignJobs();
    }

    int& numIteratorJobs()
    {
      return executor.numIteratorJobs;
    }

    int numIteratorServers() const
    {
      return executor.numIteratorServers;
    }

    int procsPerIterator() const
    {
      return executor.procsPerIterator;
    }

    int iteratorCommRank() const
    {
      return executor.iteratorCommRank;
    }

    int iteratorCommSize() const
    {
      return executor.iteratorCommSize;
    }

    int iteratorServerId() const
    {
      return executor.iteratorServerId;
    }

    bool messagePass() const
    {
      return executor.messagePass;
    }

    short iteratorScheduling() const
    {
      return executor.iteratorScheduling;
    }

    bool peerAssignJobs() const
    {
      return executor.peerAssignJobs;
    }

    size_t miPLIndex() const
    {
      return executor.miPLIndex;
    }

  private:
    StudyRuntime runtime() const
    {
      return StudyRuntime(parallelLib, outputManager);
    }

    ParallelLibrary* parallelLib;
    OutputManager* outputManager;
    IteratorExecutor executor;
  };

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

  IteratorContext create_iterator_context(
    bool peer_assign_jobs,
    int num_servers = 0,
    int procs_per_iterator = 0,
    short scheduling = DEFAULT_SCHEDULING) const
  {
    return IteratorContext(*this, peer_assign_jobs, num_servers,
                           procs_per_iterator, scheduling);
  }

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
