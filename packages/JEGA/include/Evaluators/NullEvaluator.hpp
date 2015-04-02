/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class NullEvaluator.

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

        Thu May 29 09:26:48 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the NullEvaluator class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_NULLEVALUATOR_HPP
#define JEGA_ALGORITHMS_NULLEVALUATOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmEvaluator.hpp>









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
class NullEvaluator;







/*
================================================================================
Class Definition
================================================================================
*/

/// The null object of evaluators.
/**
 * This evaluator does nothing.  It merely serves as a null object.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class NullEvaluator :
    public GeneticAlgorithmEvaluator
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
         * \return The string "null_evaluation".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This evaluator does nothing.  It is intended to serve as
            a null object for evaluators.  Use it if you do not wish
            to perform any evaluation.
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
         * \param algorithm The GA for which the new evaluator is to be used.
         * \return A new, default instance of a NullEvaluator.
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

        /// This version of Evaluate(DesignGroup) does almost nothing.
        /**
         * All it does is keep the count of evaluations current.
         * It does so by assuming every Design would need evaluation.
         * It doesn't even call Evaluate(Design) and it always returns true.
         *
         * \param group The group of un-evaluated Design class objects to
         *              evaluate.
         * \return true.  Always.
         */
        virtual
        bool
        Evaluate(
            JEGA::Utilities::DesignGroup& group
            );

        /// This version of Evaluate(Design) does almost nothing.
        /**
         * All it does is keep the count of evaluations current.  It does so by
         * assuming the Design would need evaluation. It always returns true.
         *
         * \param evalJob The description of the evaluation job to perform.
         * \return true.  Always.
         */
        virtual
        bool
        Evaluate(
            JEGA::Utilities::Design& evalJob
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

        /// Constructs an NPointBinaryCrosser for use by \a algorithm.
        /**
         * \param algorithm The GA for which this evaluator is being
         *                  constructed.
         */
        NullEvaluator(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an NullEvaluator.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        NullEvaluator(
            const NullEvaluator& copy
            );

        /// Copy constructs an NullEvaluator for use by \a algorithm.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this evaluator is being
         *                  constructed.
         */
        NullEvaluator(
            const NullEvaluator& copy,
            GeneticAlgorithm& algorithm
            );

}; // class NullEvaluator


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
#include "./inline/NullEvaluator.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_NULLEVALUATOR_HPP
