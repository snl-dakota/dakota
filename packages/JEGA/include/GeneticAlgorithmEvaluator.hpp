/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class GeneticAlgorithmEvaluator

    NOTES:

        See notes under section "Class Definition" of this file.

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
 * \brief Contains the definition of the GeneticAlgorithmEvaluator class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_GENETICALGORITHMEVALUATOR_HPP
#define JEGA_ALGORITHMS_GENETICALGORITHMEVALUATOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>
#include <../Utilities/include/DesignMultiSet.hpp>

#include <GeneticAlgorithmOperator.hpp>
#include <utilities/include/int_types.hpp>

#ifdef JEGA_THREADSAFE
#include <threads/include/mutex.hpp>
#endif





/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/
namespace JEGA
{
    namespace Utilities
    {
        class Design;
        class DesignGroup;
    }
}

/*
================================================================================
Namespace Aliases
================================================================================
*/








/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Algorithms {







/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
class GeneticAlgorithmEvaluator;




/*
================================================================================
In-Namespace File Scope Typedefs
================================================================================
*/








/*
================================================================================
Class Definition
================================================================================
*/

/// The base class for all GeneticAlgorithm evaluators.
/**
 * The evaluators is responsible for retrieving proper objective function and
 * constraint values for Designs.  It can do it however it sees fit.
 *
 * This base class provides a means of tracking the number of evaluations
 * executed.  It also requires of it's derivatives implementation of the
 * Evaluate method.  All overriding implementations of the Evaluate(Design)
 * method must call back to this base classes version after evaluation has
 * finished.
 *
 * The maximum allowable number of evaluations is extracted from the parameter
 * database using the name "method.max_function_evaluations".  It is extracted
 * as a size_type parameter.  Note that this is a parameter shared with the
 * genetic algorithm converger operator.  If it is not in the parameter
 * database, the default value as defined by DEFAULT_MAX_EVALS will be used.
 * This is required in addition to any requirements of the base class.
 */
class JEGA_SL_IEDECL GeneticAlgorithmEvaluator :
    public GeneticAlgorithmOperator
{

    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Nested Utility Class Declarations
    ============================================================================
    */
    private:

        class ThreadWorkManager;

        class ThreadManager;

    protected:


    public:

        /// The default value for the maximum allowable number of evaluations.
        static const std::size_t DEFAULT_MAX_EVALS;

        /// The default evaluation concurrency.
        static const std::size_t DEFAULT_EVAL_CONCUR;

        /// The maximum allowable evaluation concurrency.
        static const std::size_t MAX_EVAL_CONCUR;



    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The current number of evaluations completed.
        std::size_t _numEvals;

        /// The maximum allowable number of evaluations for the algorithm.
        std::size_t _maxEvals;

        /// The number of concurrent analysis to perform.
        /**
         * If in thread safe mode, this evaluator can spit out multiple
         * concurrent analysis requests.  Each request is issued by a different
         * thread.  The maximum number is limited by the number of threads that
         * are allowed on the machine.
         */
        std::size_t _evalConcur;

        /// Designs created for injection during the evaluation process.
        /**
         * Evaluators are allowed to create new candidate solutions and propose
         * them to the algorithm for inclusion in the population.  They wind
         * up as "children" and are subjected to the same fitness and selection
         * criterial that any other design is.
         *
         * Note that the injected designs must be fully evaluated.
         */
        JEGA::Utilities::DesignDVSortSet _injections;

        /// The mutext that protects the _numEvals variable in threadsafe mode.
        EDDY_DECLARE_MUTABLE_MUTEX(_numEvalsMutex)

        /// The mutext that protects the injections list in threadsafe mode.
        EDDY_DECLARE_MUTABLE_MUTEX(_injsMutex)

        ThreadManager* _tMgr;

    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


    protected:

        /// Sets the current number of evaluations completed.
        /**
         * \param num The new number of evaluations that have taken place.
         */
        void
        SetNumberEvaluations(
            std::size_t num
            );

        /// Increments the current number of evaluations completed.
        /**
         * \param by The number of evaluations to add to the current total.
         */
        void
        IncrementNumberEvaluations(
            std::size_t by = 1
            );

        /// Sets the maximum allowable number of evaluations for the algorithm.
        /**
         * Derived convergers are not required to respect this.
         *
         * \param maxEvals The new maximum allowable number of evaluations.
         */
        void
        SetMaxEvaluations(
            std::size_t maxEvals
            );

        /**
         * \brief Allows mutation of the evaluation concurrency used by this
         *        evaluator.
         *
         * This method enforces the rules of evaluation concurrency.  They
         * include the maximum allowable concurrency and a minimum of 1.
         *
         * This method also displays a verbose level log entry to inform of
         * the new value.
         *
         * \param ec The new evaluation concurrency for this evaluator.
         */
        void
        SetEvaluationConcurrency(
            std::size_t ec
            );

    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Returns the current number of evaluations completed.
        /**
         * \return The number of evaluations recorded so far for this
         *         evaluator.
         */
        std::size_t
        GetNumberEvaluations(
            ) const;

        /// Gets the maximum allowable number of evaluations for the algorithm.
        /**
         * \return The maximum allowable number of evaluations.
         */
        inline
        std::size_t
        GetMaxEvaluations(
            ) const;

        /// Allows access to the current value of evaluation concurrency.
        /**
         * \return The current evaluation concurrency used by this evaluator.
         */
        inline
        std::size_t
        GetEvaluationConcurrency(
            ) const;

        inline
        const JEGA::Utilities::DesignDVSortSet&
        GetInjections(
            ) const;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Indicates if the maximum number of evaluations has been exceeded.
        /**
         * \return True if the current number of evaluations is strictly
         *         greater than the max allowable.
         */
        inline
        bool
        IsMaxEvalsExceeded(
            ) const;

        /**
         * \brief Indicates if the maximum number of evaluations has been
         *        reached or exceeded.
         *
         * \return True if the current number of evaluations is >= the max
         *         allowable.
         */
        inline
        bool
        IsMaxEvalsReached(
            ) const;

        void
        InjectDesign(
            JEGA::Utilities::Design& des
            );

        void
        ClearInjectedDesigns(
            );

        void
        MergeInjectedDesigns(
            JEGA::Utilities::DesignGroup& into,
            bool includeUnevaluated = false,
            bool includeIllconditioned = false,
            bool includeDuplicates = false
            );

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:


        /// Looks for clones that don't need to be evaluated in \a in.
        /**
         * This method calls ResolveClone for each Design in \a in.
         *
         * \param in The group containing Designs to resolve.
         * \return The number of resolved clones.
         */
        static
        std::size_t
        ResolveClones(
            const JEGA::Utilities::DesignGroup& in
            );

        /// Tries to reslove \a des if it is a clone.
        /**
         * This method looks for an evaluated clone of \a des.  If it finds
         * one, it records the evaluation values and marks \a des as evaluated.
         *
         * This method does not do any clone detection, only resolution.
         *
         * \param des The design to attempt to evaluate by clone resolution.
         * \return true if des was resolved as a clone and false otherwise.
         */
        static
        bool
        ResolveClone(
            JEGA::Utilities::Design& des
            );

    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /// Does evaluation of each Design in \a group.
        /**
         * This method can be overridden to do additional work or to change its
         * behavior all together but in general that is not necessary and is
         * discouraged.  It may be called by the genetic algorithm.
         *
         * The default implementation is to queue up all the evaluation jobs
         * and get them executed by as many threads as is appropriate via
         * asynchronous calls the overloaded version taking an EvaluationJob.
         * Of course if thread safe mode is not in use, they all get done
         * serially by the one and only thread.  The default implementation
         * does not manage the \a _numEvals data member.  That is left to the
         * overload getting called.
         *
         * Note that evaluation in this algorithm has nothing to do with
         * fitness assessment.  This method only installs raw constraint and
         * objective function values.  Fitness assessment based on these values
         * is the job of a GeneticAlgorithmFitnessAssessor.
         *
         * \param group The group containing the Designs to be evaluated.
         * \return True if the overload returns true for all Designs.
         */
        virtual
        bool
        Evaluate(
            JEGA::Utilities::DesignGroup& group
            );

        /**
         * \brief Does evaluation of the single passed in Design.
         *
         * This method is generally not used by the architecture.  The default
         * implementation is to issue a call to the overload taking an
         * EvaluationJob from the calling thread with the current evaluation
         * count as the value passed to the job constructor.
         *
         * This method should return true if the Design is given proper
         * objective function and constraint values (either by evaluation or by
         * copying from an evaluated clone) and false if some kind of
         * error occurs.
         *
         * Note that evaluation in this algorithm has nothing to do with
         * fitness assessment.  This method only installs raw constraint and
         * objective function values.  Fitness assessment based on these values
         * is the job of a GeneticAlgorithmFitnessAssessor.
         *
         * \param des The Design object to evaluate.
         * \return True if \a des winds up with proper responses and everything
         *         is ok and false otherwise.  This base class implementation
         *            always returns true.
         */
        virtual
        bool
        Evaluate(
            JEGA::Utilities::Design& des
            ) = 0;

        /// Retrieves specific parameters using Get...FromDB methods.
        /**
         * This method is used to extract needed information for this
         * operator.  It does so using the "Get...FromDB" class
         * of methods from the GeneticAlgorithmOperator base class.
         * The return value is
         *
         * This version does nothing.
         *
         * \param db The database of parameters from which the configuration
         *           information can be retrieved.
         * \return true if the extraction completed successfully and false
         *         otherwise.
         */
        virtual
        bool
        PollForParameters(
            const JEGA::Utilities::ParameterDatabase& db
            );

        /// Returns the name of the type of this operator.
        /**
         * \return The string "Evaluator".
         */
        virtual
        std::string
        GetType(
            ) const;


    protected:

        /**
         * \brief A convenience method that does some post-evaluation
         *        bookkeeping.
         *
         * This includes incrementing the evaluation count, evaluating the
         * feasibility of the evaluated design if appropriate, etc.  This
         * should be called for each Design after it is evaluated and is
         * typically called as the last step in the Evaluate methods.  It
         * is set up so that it can generally be returned directly from those
         * methods since if evaluation failed, the design should be marked
         * illconditioned.
         *
         * \param des The Design class object that is being post-evaluated.
         * \return The conditioning status if \a des (true if des is NOT
         *         ill-conditioned and false if it is).
         */
        virtual
        bool
        PostEvaluate(
            JEGA::Utilities::Design& des
            );

    private:



    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:






    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs a GeneticAlgorithmEvaluator for use by \a algorithm.
        /**
         * \param algorithm The GA for which this evaluator is being
         *                  constructed.
         */
        GeneticAlgorithmEvaluator(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs a GeneticAlgorithmEvaluator.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        GeneticAlgorithmEvaluator(
            const GeneticAlgorithmEvaluator& copy
            );

        /**
         * \brief Copy constructs a GeneticAlgorithmEvaluator for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this evaluator is being
         *                  constructed.
         */
        GeneticAlgorithmEvaluator(
            const GeneticAlgorithmEvaluator& copy,
            GeneticAlgorithm& algorithm
            );

        ~GeneticAlgorithmEvaluator(
            );

}; // class GeneticAlgorithmEvaluator


/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA







/*
================================================================================
Include Inlined Methods File
================================================================================
*/
#include "./inline/GeneticAlgorithmEvaluator.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_GENETICALGORITHMEVALUATOR_HPP
