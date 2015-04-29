/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class GeneticAlgorithmEvaluator

    NOTES:

        See notes of GeneticAlgorithmEvaluator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue May 20 10:41:52 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the GeneticAlgorithmEvaluator class.
 */






/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <list>
#include <limits>
#include <numeric>
#include <algorithm>
#include <utilities/include/Math.hpp>
#include <GeneticAlgorithmEvaluator.hpp>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>

#ifdef JEGA_THREADSAFE
#include <threads/include/thread.hpp>
#include <threads/include/condition.hpp>
#include <threads/include/mutex_lock.hpp>
#include <threads/include/mutexed_variable.hpp>
#endif

// Not all implementations of pthreads define this constant.  So use a default
// large value if not.
#ifndef PTHREAD_THREADS_MAX
#define PTHREAD_THREADS_MAX 1024
#endif


/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA;
using namespace JEGA::Logging;
using namespace JEGA::Utilities;
using namespace eddy::utilities;






/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Algorithms {




/*
================================================================================
Nested Utility Class Implementations
================================================================================
*/
class GeneticAlgorithmEvaluator::ThreadWorkManager
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        GeneticAlgorithmEvaluator& _evaler;

        bool _combinedResult;

        /// The current iterator represented by this object.
        DesignDVSortSet::const_iterator _curr;

        /// The end of the job queue stored for the is_end method.
        const DesignDVSortSet::const_iterator _end;

        size_t _offset;

        size_t _numSent;

        size_t _numSkipped;

        size_t _maxEvals;

        /// The mutex used to protect \a _curr in threadsafe mode.
        EDDY_DECLARE_MUTABLE_MUTEX(_itMutex)

        EDDY_DECLARE_MUTABLE_MUTEX(_resMutex)

    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        inline
        GeneticAlgorithmEvaluator&
        evaler(
            )
        {
            return this->_evaler;
        }

        Design*
        get_next_design(
            )
        {
            EDDY_SCOPEDLOCK(l, this->_itMutex)

            // If we are at the end of our work, just return a null pointer.
            if(this->_curr == this->_end) return 0x0;

            // If we have exceeded the evaluators maximum number of evaluations,
            // then we don't send out a design.  The rest are all skipped.
            if((this->_numSent + this->_offset) >= this->_maxEvals)
            {
                for(; this->_curr!=this->_end; ++this->_curr)
                {
                    Design* des = *this->_curr;
                    if(des->IsEvaluated()) continue;
                    ++this->_numSkipped;
                    des->SetEvaluated(true);
                    des->SetIllconditioned(true);
                }
                if(this->_numSkipped > 0) this->add_result(false);
                return 0x0;
            }

            // Otherwise, we do send one out if we can find one.  The design at
            // curr has not yet been sent so it is the first candidate.
            while((*this->_curr)->IsEvaluated())
                if((++this->_curr) == this->_end) return 0x0;

            // Here, we have a design that needs to be evaluated.
            // Indicate that we have sent another design out and return it.
            ++this->_numSent;
            return *(this->_curr++);
        }

        inline
        bool
        add_result(
            bool result
            )
        {
            EDDY_FUNC_DEBUGSCOPE
            EDDY_SCOPEDLOCK(l, this->_resMutex)
            return (this->_combinedResult &= result);
        }

        inline
        size_t
        num_skipped(
            )
        {
            return this->_numSkipped;
        }

        inline
        bool
        final_result(
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            EDDY_SCOPEDLOCK(l, this->_resMutex)
            return this->_combinedResult;
        }

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /**
         * \brief Constructs a ThreadWorkManager to traverse the supplied
         *        range.
         *
         * \a begin and \a end must come from the same job queue instance.
         *
         * \param begin An iterator to the first EvaluationJob to do.
         * \param end An iterator to one past the last EvaluationJob to do.
         */
        ThreadWorkManager(
            GeneticAlgorithmEvaluator& evaler,
            const DesignDVSortSet& dvSort
            ) :
                _evaler(evaler),
                _combinedResult(true),
                _curr(dvSort.begin()),
                _end(dvSort.end()),
                _offset(evaler.GetNumberEvaluations()),
                _numSent(0),
                _numSkipped(0),
                _maxEvals(evaler.GetMaxEvaluations()) EDDY_COMMA_IF_THREADSAFE
                EDDY_INIT_MUTEX(_itMutex, PTHREAD_MUTEX_RECURSIVE)
                    EDDY_COMMA_IF_THREADSAFE
                EDDY_INIT_MUTEX(_resMutex, PTHREAD_MUTEX_RECURSIVE)
        {
            EDDY_FUNC_DEBUGSCOPE
        }

}; // GeneticAlgorithmEvaluator::ThreadWorkManager



class GeneticAlgorithmEvaluator::ThreadManager
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        ThreadWorkManager* _twm;

        size_t _threadCt;

        bool _exit;

        JEGA_IF_THREADSAFE(pthread_t* _threads;)

        JEGA_IF_THREADSAFE(eddy::threads::condition _workCond;)

        EDDY_DECLARE_MUTABLE_MUTEX(_workLock);

        JEGA_IF_THREADSAFE(eddy::threads::condition _completeCond;)

        JEGA_IF_THREADSAFE(eddy::threads::mutexed_variable<size_t> _numComplete);

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        void
        ExecuteEvaluations(
            ThreadWorkManager& twm
            )
        {
            EDDY_FUNC_DEBUGSCOPE
            this->_twm = &twm;
            JEGA_IF_THREADSAFE(if(this->_threadCt > 1) {)
                EDDY_SCOPEDLOCK(l, this->_workLock);
                JEGA_IF_THREADSAFE(this->_workCond.notify_all();)
                JEGA_IF_NO_THREADSAFE(ExecuteJobs(this);)
                JEGA_IF_THREADSAFE(this->_completeCond.wait(l);)
            JEGA_IF_THREADSAFE(} else) ExecuteJobsNoThread(this);
            this->_twm = 0x0;
        }

        void
        WaitForAllToComplete(
            )
        {
            EDDY_FUNC_DEBUGSCOPE
            JEGA_IF_THREADSAFE(if(this->_threadCt < 2) return;)
            EDDY_SCOPEDLOCK(l, this->_workLock);
            JEGA_IF_THREADSAFE(this->_completeCond.wait(l);)
        }

        void
        ResetThreadCount(
            size_t newTC
            )
        {
            EDDY_FUNC_DEBUGSCOPE

#ifdef JEGA_THREADSAFE

            if(newTC == this->_threadCt) return;
            ThreadWorkManager* twm = this->_twm;
            this->_twm = 0x0;

            if(this->_threadCt > 1)
            {
                this->_exit = true;
                this->_workCond.notify_all();
                for(size_t i=0; i<this->_threadCt; ++i)
                    pthread_join(this->_threads[i], 0x0);
                this->_exit = false;
            }

            this->_threadCt = newTC;
            this->_twm = twm;
            this->CreateThreads();
#else
            this->_threadCt = newTC;
#endif
        }

    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:

#ifdef JEGA_THREADSAFE

        void
        mark_done(
            )
        {
            EDDY_FUNC_DEBUGSCOPE

            size_t oldVal = this->_numComplete.lock();
            size_t newVal = this->_numComplete.set(oldVal + 1);
            this->_numComplete.unlock();

            if(newVal >= this->_threadCt) {
                this->_numComplete.set(0);
                this->_completeCond.notify_all();
            }
        }

        static
        void*
        ExecuteJobs(
            void* data
            )
        {
            EDDY_FUNC_DEBUGSCOPE

            ThreadManager& tm = *static_cast<ThreadManager*>(data);

            while(!tm._exit)
            {
                if(tm._twm != 0x0) for(
                    Design* des=tm._twm->get_next_design();
                    des!=0x0; des=tm._twm->get_next_design()
                    ) tm._twm->add_result(tm._twm->evaler().Evaluate(*des));

                EDDY_SCOPEDLOCK(l, tm._workLock);
                if(tm._twm != 0x0) tm.mark_done();
                tm._workCond.wait(l);
            }
            return data;
        };

        void
        CreateThreads(
            )
        {
            delete [] this->_threads;
            if(this->_threadCt < 2) return;
            this->_threads = new pthread_t[this->_threadCt];

            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

            for(size_t i=0; i<this->_threadCt; ++i) pthread_create(
                &(this->_threads[i]), &attr, &this->ExecuteJobs, this
                );

            pthread_attr_destroy(&attr);

            // Hope is that all threads will get to their wait.
            eddy::threads::thread::yield();
        }

#else

        void
        mark_done(
            )
        {
        }

        static
        void*
        ExecuteJobs(
            void* data
            )
        {
            EDDY_FUNC_DEBUGSCOPE
            return ExecuteJobsNoThread(data);
        }

        void
        CreateThreads(
            )
        {
        }

#endif

        static
        void*
        ExecuteJobsNoThread(
            void* data
            )
        {
            EDDY_FUNC_DEBUGSCOPE

            ThreadManager& tm = *static_cast<ThreadManager*>(data);

            for(Design* des=tm._twm->get_next_design();
                des!=0x0;
                des=tm._twm->get_next_design())
                    tm._twm->add_result(tm._twm->evaler().Evaluate(*des));

            return data;
        };

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        ThreadManager(
            size_t iThreadCt
            ) :
                _twm(0x0),
                _threadCt(iThreadCt),
                _exit(false) EDDY_COMMA_IF_THREADSAFE
                JEGA_IF_THREADSAFE(_threads(0x0)) EDDY_COMMA_IF_THREADSAFE
                JEGA_IF_THREADSAFE(_workCond()) EDDY_COMMA_IF_THREADSAFE
                EDDY_INIT_MUTEX(_workLock, PTHREAD_MUTEX_RECURSIVE)
                    EDDY_COMMA_IF_THREADSAFE
                JEGA_IF_THREADSAFE(_completeCond()) EDDY_COMMA_IF_THREADSAFE
                JEGA_IF_THREADSAFE(_numComplete(0))
        {
            this->CreateThreads();
        }

        ~ThreadManager(
            )
        {
            this->_twm = 0x0;

#ifdef JEGA_THREADSAFE
            if(this->_threadCt > 1)
            {
                this->_exit = true;
                this->_workCond.notify_all();
                for(size_t i=0; i<this->_threadCt; ++i)
                    pthread_join(this->_threads[i], 0x0);
                delete [] this->_threads;
            }
#endif
        }
};

/*
================================================================================
Static Member Data Definitions
================================================================================
*/

const std::size_t GeneticAlgorithmEvaluator::DEFAULT_MAX_EVALS(
    numeric_limits<std::size_t>::max()
    );

const std::size_t GeneticAlgorithmEvaluator::DEFAULT_EVAL_CONCUR(1);

const std::size_t GeneticAlgorithmEvaluator::MAX_EVAL_CONCUR(
    JEGA_IF_THREADSAFE(PTHREAD_THREADS_MAX) JEGA_IF_NO_THREADSAFE(1)
    );



/*
================================================================================
Mutators
================================================================================
*/
void
GeneticAlgorithmEvaluator::SetMaxEvaluations(
    std::size_t maxEvals
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_maxEvals = maxEvals;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": Maximum allowable evaluations now = ")
            << this->_maxEvals
        )
}

void
GeneticAlgorithmEvaluator::SetEvaluationConcurrency(
    std::size_t ec
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGAIFLOG_II(ec < 1, this->GetLogger(), lquiet(), this,
        text_entry(lquiet(), this->GetName() + ": Attempt to assign the "
            "evaluation concurrency to 0.  1 is the minimum evaluation "
            "concurency.  Using the value of 1 instead.")
        )

    JEGAIFLOG_II(ec > MAX_EVAL_CONCUR, this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(), this->GetName() + ": Attempt to assign the "
            "evalauation concurrency to a value of ") << ec << " which is "
            "greater than the maximum allowable value of " << MAX_EVAL_CONCUR
            << ". Using the max value."
        )

    this->_evalConcur =
        Math::Min<size_t>(Math::Max<size_t>(ec, 1), MAX_EVAL_CONCUR);

    this->_tMgr->ResetThreadCount(this->_evalConcur);

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() +
            ": The evaluation concurrency is now ") << this->_evalConcur
        )
}


void
GeneticAlgorithmEvaluator::SetNumberEvaluations(
    std::size_t num
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_SCOPEDLOCK(l, this->_numEvalsMutex)
    this->_numEvals = num;
}

void
GeneticAlgorithmEvaluator::IncrementNumberEvaluations(
    std::size_t by
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_SCOPEDLOCK(l, this->_numEvalsMutex)
    this->_numEvals += by;
}








/*
================================================================================
Accessors
================================================================================
*/

std::size_t
GeneticAlgorithmEvaluator::GetNumberEvaluations(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_SCOPEDLOCK(l, this->_numEvalsMutex)
    return this->_numEvals;
}








/*
================================================================================
Public Methods
================================================================================
*/
void
GeneticAlgorithmEvaluator::InjectDesign(
    Design& des
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_SCOPEDLOCK(l, this->_injsMutex)
    this->_injections.insert(&des);
}

void
GeneticAlgorithmEvaluator::ClearInjectedDesigns(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_SCOPEDLOCK(l, this->_injsMutex)
    this->GetDesignTarget().TakeDesigns(this->_injections);
    this->_injections.clear();
}

void
GeneticAlgorithmEvaluator::MergeInjectedDesigns(
    JEGA::Utilities::DesignGroup& into,
    bool includeUnevaluated,
    bool includeIllconditioned,
    bool includeDuplicates
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Merge in any designs injected via the evaluator.  They get merged
    // into the children and must already be evaluated.
    EDDY_SCOPEDLOCK(l, this->_injsMutex)
    JEGA_LOGGING_IF_ON(const std::size_t oisze = this->_injections.size();)
    const DesignDVSortSet& dvSort = into.GetDVSortContainer();

    for(DesignDVSortSet::iterator it(this->_injections.begin());
        it!=this->_injections.end();)
    {
        Design* des = *it;

        if(!includeIllconditioned && des->IsIllconditioned())
        {
            JEGALOG_II(this->GetLogger(), lquiet(), this,
                text_entry(
                    lquiet(),
                    this->GetName() + ": encountered and skipped an "
                    "illconditioned injection design.  The design was not "
                    "merged into the group."
                    )
                )
            ++it;
        }
        else if(!includeUnevaluated && !des->IsEvaluated())
        {
            JEGALOG_II(this->GetLogger(), lquiet(), this,
                text_entry(
                    lquiet(),
                    this->GetName() + ": encountered and skipped an "
                    "unevaluated injection design.  The design was not "
                    "merged into the group."
                    )
                )
            ++it;
        }
        else if(!includeDuplicates && dvSort.test_for_clone(des) != dvSort.end())
        {
            JEGALOG_II(this->GetLogger(), lquiet(), this,
                text_entry(
                    lquiet(),
                    this->GetName() + ": encountered and skipped a "
                    "duplicate injection design.  The design was not "
                    "merged into the group."
                    )
                )
            ++it;
        }
        else
        {
            into.Insert(des);
            this->_injections.erase(it++);
        }
    }

    JEGAIFLOG_CF_II(this->_injections.size() != oisze, this->GetLogger(),
        lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": added ")
            << oisze - this->_injections.size()
            << " injected designs into the supplied set.  Skipped "
            << this->_injections.size() << '.'
        )
}







/*
================================================================================
Subclass Visible Methods
================================================================================
*/


std::size_t
GeneticAlgorithmEvaluator::ResolveClones(
    const DesignGroup& in
    )
{
    EDDY_FUNC_DEBUGSCOPE
    std::size_t ret = 0;
    for(DesignDVSortSet::const_iterator it(in.BeginDV()); it!=in.EndDV(); ++it)
        ret += ResolveClone(**it) ? 1 : 0;
    return ret;
}

bool
GeneticAlgorithmEvaluator::ResolveClone(
    Design& des
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // This method does not do any clone detection, only resolution
    // which means that it copies response values from an evaluated
    // clone if it can find one.

    // if this design is already evaluated or isn't a clone, stop.
    if(des.IsEvaluated() || !des.IsCloned()) return false;

    // this will be the pointer to a clone if one is found.
    Design* clone = 0x0;

    // look beyond "des" in the clone list for an evaluated Design
    for(clone=des.GetNextClone();
        (clone!=0x0) && !clone->IsEvaluated();
        clone = clone->GetNextClone());

    // If we didn't find one, look at the clones prior in the list.
    if(clone == 0x0) for(clone=des.GetPreviousClone();
        (clone!=0x0) && !clone->IsEvaluated();
        clone = clone->GetPreviousClone());

    // if we didn't make it to the end, we found a clone.
    // record the values, mark "des" evaluated, and get out.
    if(clone != 0x0)
    {
        des.CopyResponses(*clone);
        des.SetEvaluated(true);
        return true;
    }

    // if we make it here, des could not be resolved this way.
    return false;
}

/*
================================================================================
Subclass Overridable Methods
================================================================================
*/
bool
GeneticAlgorithmEvaluator::Evaluate(
    DesignGroup& group
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(ldebug(), this->GetName() + ": Group evaluator in use.")
        )

    // first resolve clones
    JEGA_LOGGING_IF_ON(size_t nres =) this->ResolveClones(group);

    JEGAIFLOG_II(nres > 0, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": able to avoid ") << nres
            << " evaluations by copying responses from cloned designs."
        )

    // now that we have our queue, get them evaluated.
    ThreadWorkManager twm(*this, group.GetDVSortContainer());
    this->_tMgr->ExecuteEvaluations(twm);

    JEGA_LOGGING_IF_ON(size_t nskipped = twm.num_skipped();)

    JEGAIFLOG_II(nskipped > 0, this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(), this->GetName() + ": skipped evaluation of ")
            << nskipped << " designs because the maximum number of "
               "evaluations was reached.  They were marked illconditioned."
        )

    return twm.final_result();
}

bool
GeneticAlgorithmEvaluator::PollForParameters(
    const JEGA::Utilities::ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    bool success = ParameterExtractor::GetSizeTypeFromDB(
        db, "method.max_function_evaluations", this->_maxEvals
        );

    // If we did not find the max iterations, warn about it and use the default
    // value.  Note that if !success, then maxGens is still equal to _maxGens
    JEGAIFLOG_II(!success, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The maximum allowable "
            "number of evaluations was not found in the parameter "
            "database.  Using the current value of ") << this->_maxEvals
        )

    // now go ahead and set it.
    this->SetMaxEvaluations(this->_maxEvals);

    success = ParameterExtractor::GetSizeTypeFromDB(
        db, "method.jega.eval_concurrency", this->_evalConcur
        );

    JEGAIFLOG_II(!success, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The evaluation "
            "concurrency was not found in the parameter database.  Using the "
            "current value of ") << this->_evalConcur
        )

    this->SetEvaluationConcurrency(this->_evalConcur);

    return true;
}

string
GeneticAlgorithmEvaluator::GetType(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return "Evaluator";
}

bool
GeneticAlgorithmEvaluator::PostEvaluate(
    JEGA::Utilities::Design& des
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // need the target to get the design variable information objects.
    const DesignTarget& target = des.GetDesignTarget();

    des.SetEvaluated(true);

    if(!des.IsIllconditioned())
    {
        target.CheckFeasibility(des);
        target.RecordAllConstraintViolations(des);
    }

    this->IncrementNumberEvaluations();

    return !des.IsIllconditioned();
}


/*
================================================================================
Private Methods
================================================================================
*/





/*
================================================================================
Structors
================================================================================
*/
GeneticAlgorithmEvaluator::GeneticAlgorithmEvaluator(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmOperator(algorithm),
        _numEvals(0),
        _maxEvals(DEFAULT_MAX_EVALS),
        _evalConcur(DEFAULT_EVAL_CONCUR),
        _tMgr(new ThreadManager(DEFAULT_EVAL_CONCUR)) EDDY_COMMA_IF_THREADSAFE
        EDDY_INIT_MUTEX(_injsMutex, PTHREAD_MUTEX_RECURSIVE)EDDY_COMMA_IF_THREADSAFE
        EDDY_INIT_MUTEX(_numEvalsMutex, PTHREAD_MUTEX_RECURSIVE) 
{
    EDDY_FUNC_DEBUGSCOPE
}

GeneticAlgorithmEvaluator::GeneticAlgorithmEvaluator(
    const GeneticAlgorithmEvaluator& copy
    ) :
        GeneticAlgorithmOperator(copy),
        _numEvals(copy._numEvals),
        _maxEvals(copy._maxEvals),
        _evalConcur(copy._evalConcur),
        _tMgr(new ThreadManager(copy._evalConcur)) EDDY_COMMA_IF_THREADSAFE
        EDDY_INIT_MUTEX(_injsMutex, PTHREAD_MUTEX_RECURSIVE) EDDY_COMMA_IF_THREADSAFE
        EDDY_INIT_MUTEX(_numEvalsMutex, PTHREAD_MUTEX_RECURSIVE)
{
    EDDY_FUNC_DEBUGSCOPE
}

GeneticAlgorithmEvaluator::GeneticAlgorithmEvaluator(
    const GeneticAlgorithmEvaluator& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmOperator(copy, algorithm),
        _numEvals(copy._numEvals),
        _maxEvals(copy._maxEvals),
        _evalConcur(copy._evalConcur),
        _tMgr(new ThreadManager(copy._evalConcur)) EDDY_COMMA_IF_THREADSAFE
        EDDY_INIT_MUTEX(_injsMutex, PTHREAD_MUTEX_RECURSIVE) EDDY_COMMA_IF_THREADSAFE
        EDDY_INIT_MUTEX(_numEvalsMutex, PTHREAD_MUTEX_RECURSIVE)
{
    EDDY_FUNC_DEBUGSCOPE
}

GeneticAlgorithmEvaluator::~GeneticAlgorithmEvaluator(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    delete this->_tMgr;
}




/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA
