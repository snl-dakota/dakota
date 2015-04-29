/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class GeneticAlgorithmConverger

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

        Tue May 20 11:25:12 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the GeneticAlgorithmConverger class.
 */





/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_GENETICALGORITHMCONVERGER_HPP
#define JEGA_ALGORITHMS_GENETICALGORITHMCONVERGER_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmOperator.hpp>
#include <utilities/include/int_types.hpp>







/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/
namespace JEGA
{
    namespace Utilities
    {
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
class FitnessRecord;
class GeneticAlgorithmConverger;





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

/// The base class for all GeneticAlgorithm convergers.
/**
 * The converger is responsible for checking for convergence of an algorithm.
 * It can do so by any means it wishes.
 *
 * This base class provides a means of checking for overrun of the number of
 * generations, overrun of the number of evaluations, and overrun of the amount
 * of wall clock time..  Derived versions are free to use these methods but
 * don't have to.  Derived versions are required to implement the
 * CheckConvergence method.
 *
 * The maximum allowable generations, maximum allowable evaluations, and maximum
 * allowable wall clock time are extracted from the parameter database using the
 * names "method.max_iterations", "method.max_function_evaluations", and
 * "method.max_time" respectively.  The evaluations and iterations are extracted
 * as size type parameters.  The time is a double.  If they are not in the
 * parameter database, the default values as defined by DEFAULT_MAX_GENS,
 * DEFAULT_MAX_EVALS, and DEFAULT_MAX_TIME respectively will be used.  These are
 * required in addition to any requirements of the base class.
 */
class JEGA_SL_IEDECL GeneticAlgorithmConverger :
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
    Member Data Declarations
    ============================================================================
    */
    public:

        /// The default value for the maximum allowable number of generations.
        static const std::size_t DEFAULT_MAX_GENS;

        /// The default value for the maximum allowable number of evaluations.
        static const std::size_t DEFAULT_MAX_EVALS;

        /// The default value for the maximum allowable amount of time
        // (in seconds).
        static const double DEFAULT_MAX_TIME;

    private:

        /// The maximum allowable number of generations for the algorithm.
        std::size_t _maxGens;

        /// The maximum allowable number of evaluations for the algorithm.
        std::size_t _maxEvals;

        /// The maximum allowable amount of time (seconds) for the algorithm.
        double _maxTime;

        /// True if convergence has been achieved and false otherwise.
        /**
         * A flag indicating the state of convergence as converged or not.
         * A value of true indicates that convergence has been reached.
         */
        bool _converged;



    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Sets the maximum allowable number of generations for the algorithm.
        /**
         * Derived convergers are not required to respect this.
         *
         * This method enters a verbose level log entry informing of the new
         * maximum number of generations.
         *
         * \param maxGens The new maximum allowable number of generations.
         */
        void
        SetMaxGenerations(
            std::size_t maxGens
            );

        /// Sets the maximum allowable number of evaluations for the algorithm.
        /**
         * Derived convergers are not required to respect this.
         *
         * This method enters a verbose level log entry informing of the new
         * maximum number of evaluations.
         *
         * \param maxEvals The new maximum allowable number of evaluations.
         */
        void
        SetMaxEvaluations(
            std::size_t maxEvals
            );

        /// Sets the maximum allowable amount of time for the algorithm.
        /**
         * Derived convergers are not required to respect this.
         *
         * This method enters a verbose level log entry informing of the new
         * maximum amount of time.
         *
         * \param maxEvals The new maximum allowable amount of time in seconds.
         */
        void
        SetMaxTime(
            double maxTime
            );

    protected:

        /// Sets the converged flag for this converger.
        /**
         * This method enters a verbose level log entry informing that
         * convergence has been attained if \a lval is true.
         *
         * \param lval The new logical value for whether or not convergence
         *             has been obtained.  True if it has and false if not.
         */
        void
        SetConverged(
            bool lval = true
            );

    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Gets the maximum allowable number of generations for the algorithm.
        /**
         * \return The maximum allowable number of generations.
         */
        inline
        std::size_t
        GetMaxGenerations(
            ) const;

        /// Gets the maximum allowable number of evaluations for the algorithm.
        /**
         * \return The maximum allowable number of evaluations.
         */
        inline
        std::size_t
        GetMaxEvaluations(
            ) const;

        /// Gets the maximum allowable amount of time for the algorithm.
        /**
         * \return The maximum allowable amount of time in seconds.
         */
        inline
        double
        GetMaxTime(
            ) const;

        /// Gets the converged flag for this converger.
        /**
         * \return Whether or not this converger has reported convergence.
         */
        inline
        bool
        GetConverged(
            ) const;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Indicates if the maximum number of generations has been exceeded.
        /**
         * \return True if the current number of generations is strictly
         *         greater than the max allowable.
         */
        bool
        IsMaxGensExceeded(
            ) const;

        /// Indicates if the maximum number of evaluations has been exceeded.
        /**
         * \return True if the current number of evaluations is strictly
         *         greater than the max allowable.
         */
        bool
        IsMaxEvalsExceeded(
            ) const;

        /// Indicates if the maximum amount of time has been exceeded.
        /**
         * \return True if the current amount of time in seconds is strictly
         *         greater than the max allowable.
         */
        bool
        IsMaxTimeExceeded(
            ) const;

        /**
         * \brief Indicates if the maximum number of generations has been
         *        reached or exceeded.
         *
         * \return True if the current generation number is >= the max
         *         allowable.
         */
        bool
        IsMaxGensReached(
            ) const;

        /**
         * \brief Indicates if the maximum number of evaluations has been
         *        reached or exceeded.
         *
         * \return True if the current number of evaluations is >= the max
         *         allowable.
         */
        bool
        IsMaxEvalsReached(
            ) const;

        /**
         * \brief Indicates if the maximum amount of time has been reached or
         *        exceeded.
         *
         * \return True if the current amount of time is >= the max allowable.
         */
        bool
        IsMaxTimeReached(
            ) const;



    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:





    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /**
         * \brief This is the method in which derived convergers should do
         *        whatever they do.
         *
         * It is called by the genetic algorithm periodically.  Required
         * information can be retrieved through the
         * GeneticAlgorithmOperator::GetAlgorithm() base class method.
         *
         * The method can do whatever you wish but should return
         * true to indicate convergence and flase to indicate non-convergence.
         * It should also use the \a SetConverged method of this class to
         * record the result.
         *
         * \param group The group to use in the convergence check.
         * \param fitnesses The fitnesses of the designs in \a group.
         * \return true if convergence has been achieved and false otherwise.
         */
        virtual
        bool
        CheckConvergence(
            const JEGA::Utilities::DesignGroup& group,
            const FitnessRecord& fitnesses
            ) = 0;

        virtual
        bool
        CheckConvergence(
            );

        /// Retrieves specific parameters using Get...FromDB methods.
        /**
         * This method is used to extract needed information for this operator.
         * It does so using the "Get...FromDB" class of methods from the
         * GeneticAlgorithmOperator base class.
         *
         * This version extracts the maximum number of generations and
         * the maximum number of evaluations.  Derived classes may choose to
         * use this information or not.
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
         * \return The string "Converger".
         */
        virtual
        std::string
        GetType(
            ) const;

    protected:


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

        /// Constructs a GeneticAlgorithmConverger for use by \a algorithm.
        /**
         * \param algorithm The GA for which this converger is being
         *                  constructed.
         */
        GeneticAlgorithmConverger(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs a GeneticAlgorithmConverger.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        GeneticAlgorithmConverger(
            const GeneticAlgorithmConverger& copy
            );

        /**
         * \brief Copy constructs a GeneticAlgorithmConverger for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this converger is being
         *                  constructed.
         */
        GeneticAlgorithmConverger(
            const GeneticAlgorithmConverger& copy,
            GeneticAlgorithm& algorithm
            );

}; // class GeneticAlgorithmConverger


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
#include "./inline/GeneticAlgorithmConverger.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_GENETICALGORITHMCONVERGER_HPP
