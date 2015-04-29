/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class MaximumEvaluationConverger.

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

        Wed Jun 11 07:13:13 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the MaximumEvaluationConverger class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_MAXIMUMEVALUATIONCONVERGER_HPP
#define JEGA_ALGORITHMS_MAXIMUMEVALUATIONCONVERGER_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmConverger.hpp>









/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/









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
class MaximumEvaluationConverger;







/*
================================================================================
Class Definition
================================================================================
*/

/// Indicates convergence by exceeding maximum evaluations.
/**
 * This converger checks the current number of evaluations of the
 * GeneticAlgorithm and returns true if that value is greater than
 * the specified maximum number of evaluations.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class MaximumEvaluationConverger :
    public GeneticAlgorithmConverger
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:





    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:





    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:





    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:


        /// Returns the proper name of this operator.
        /**
         * \return The string "max_evaluations".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This converger returns true if the maximum allowable
            number of evaluations has been reached or exceeded.
           \endverbatim.
         *
         * \return A description of the operation of this operator.
         */
        static
        const std::string&
        Description(
            );

        /**
         * \brief Returns a new instance of this operator class for use by
         *        \a algorithm.
         *
         * \param algorithm The GA for which the new converger is to be used.
         * \return A new, default instance of a MaximumEvaluationConverger.
         */
        static
        GeneticAlgorithmOperator*
        Create(
            GeneticAlgorithm& algorithm
            );


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

        /// Tests for convergence
        /**
         * This method checks to see if the maximum number of evaluations
         * has been reached (or exceeded) and returns true if so.
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
            );

        virtual
        bool
        CheckConvergence(
            );

        /// Returns the proper name of this operator.
        /**
         * \return See Name().
         */
        virtual
        std::string
        GetName(
            ) const;

        /// Returns a full description of what this operator does and how.
        /**
         * \return See Description().
         */
        virtual
        std::string
        GetDescription(
            ) const;

        /**
         * \brief Creates and returns a pointer to an exact duplicate of this
         *        operator.
         *
         * \param algorithm The GA for which the clone is being created.
         * \return A clone of this operator.
         */
        virtual
        GeneticAlgorithmOperator*
        Clone(
            GeneticAlgorithm& algorithm
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

        /**
         * \brief Constructs a MaximumEvaluationConverger for use by \a
         *         algorithm.
         *
         * \param algorithm The GA for which the new converger is to be used.
         */
        MaximumEvaluationConverger(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs a MaximumEvaluationConverger.
        /**
         * \param copy The converger from which properties are to be duplicated
         *             into this.
         */
        MaximumEvaluationConverger(
            const MaximumEvaluationConverger& copy
            );

        /**
         * \brief Copy constructs a MaximumEvaluationConverger for use by
         *        \a algorithm.
         *
         * \param copy The converger from which properties are to be duplicated
         *             into this.
         * \param algorithm The GA for which the new converger is to be used.
         */
        MaximumEvaluationConverger(
            const MaximumEvaluationConverger& copy,
            GeneticAlgorithm& algorithm
            );


}; // class MaximumEvaluationConverger


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
#include "./inline/MaximumEvaluationConverger.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_MAXIMUMEVALUATIONCONVERGER_HPP
