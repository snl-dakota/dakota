/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class NullMainLoop.

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

        Thu May 29 09:26:34 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the NullMainLoop class.
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_NULLMAINLOOP_HPP
#define JEGA_ALGORITHMS_NULLMAINLOOP_HPP







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
class NullMainLoop;







/*
================================================================================
Class Definition
================================================================================
*/

/// The null object of main loops.
/**
 * This main loop does nothing.  It merely serves as a null object.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class NullMainLoop :
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
         * \return The string "null_main_loop".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This main loop does nothing.  It is intended to serve as
            a null object for main loops.  Use it if you do not wish
            to perform a main loop.
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
         * \return A new, default instance of a NullMainLoop.
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

        /// This version of RunGeneration does almost nothing
        /**
         * All it does is keep the generation count.  It always returns true
         * meaning that it is ok to call it again.
         *
         * \return true. Always.
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


        /// Constructs an NullMainLoop for use by \a algorithm.
        /**
         * \param algorithm The GA for which this main loop is
         *                  being constructed.
         */
        NullMainLoop(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an NullMainLoop.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        NullMainLoop(
            const NullMainLoop& copy
            );

        /// Copy constructs an NullMainLoop for use by \a algorithm.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this main loop is
         *                  being constructed.
         */
        NullMainLoop(
            const NullMainLoop& copy,
            GeneticAlgorithm& algorithm
            );

}; // class NullMainLoop


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
#include "./inline/NullMainLoop.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_NULLMAINLOOP_HPP
