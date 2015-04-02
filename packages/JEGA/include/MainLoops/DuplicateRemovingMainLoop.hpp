/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class DuplicateRemovingMainLoop.

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

        Thu Jun 12 07:58:21 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the DuplicateRemovingMainLoop class.
 */





/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_DUPLICATEREMOVINGMAINLOOP_HPP
#define JEGA_ALGORITHMS_DUPLICATEREMOVINGMAINLOOP_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmMainLoop.hpp>







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
class DuplicateRemovingMainLoop;







/*
================================================================================
Class Definition
================================================================================
*/

/// Executes the operators in typical order but adds duplicate removal.
/**
 * This main loop operator behaves exactly as the StandardMainLoop with two
 * exceptions.  First, prior to fitness assessment, all offspring Designs that
 * duplicate a population member or another child Design are removed.  Second,
 * after selection, any non-unique population members are flushed.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class DuplicateRemovingMainLoop :
    public GeneticAlgorithmMainLoop
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
         * \return The string "duplicate_free".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This main loop operator behaves exactly as the StandardMainLoop
            with two exceptions.  First, prior to fitness assessment, all
            offspring Designs that duplicate a population member or another
            child Design are removed.  Second, after selection, any non-unique
            population members are flushed.
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
         * \param algorithm The GA for which the new main loop is to be used.
         * \return A new, default instance of a DuplicateRemovingMainLoop.
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

        /// Performs one trip through the main loop.
        /**
         * The operations are executed in the order described in the
         * documetation of this class.
         *
         * \return True if this loop wishes to be called again and false
         *         otherwise which means that convergence has occurred.
         */
        virtual
        bool
        RunGeneration(
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


        /// Constructs an DuplicateRemovingMainLoop for use by \a algorithm.
        /**
         * \param algorithm The GA for which this main loop is
         *                  being constructed.
         */
        DuplicateRemovingMainLoop(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an DuplicateRemovingMainLoop.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        DuplicateRemovingMainLoop(
            const DuplicateRemovingMainLoop& copy
            );

        /**
         * \brief Copy constructs an DuplicateRemovingMainLoop for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this main loop is
         *                  being constructed.
         */
        DuplicateRemovingMainLoop(
            const DuplicateRemovingMainLoop& copy,
            GeneticAlgorithm& algorithm
            );



}; // class DuplicateRemovingMainLoop


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
#include "./inline/DuplicateRemovingMainLoop.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_DUPLICATEREMOVINGMAINLOOP_HPP
