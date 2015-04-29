/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class NullConverger.

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

        Thu May 29 09:25:59 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the NullConverger class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_NULLCONVERGER_HPP
#define JEGA_ALGORITHMS_NULLCONVERGER_HPP







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
class NullConverger;







/*
================================================================================
Class Definition
================================================================================
*/

/// The null object of convergers.
/**
 * This converger does nothing.  It merely serves as a null object.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class NullConverger :
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
         * \return The string "null_convergence".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This converger does nothing.  It is intended to serve as
            a null object for convergers.  Use it if you do not wish
            to check for convergence.
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
         * \return A new, default instance of a NullConverger.
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

        /**
         * \brief This version of the CheckConvergence does nothing.
         *        It always returns false (not converged);
         *
         * \param group The group to use in the convergence check.
         * \param fitnesses The fitnesses of the designs in \a group.
         * \return false.  Always.
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
         * \brief Constructs a NullConverger for use by \a algorithm.
         *
         * \param algorithm The GA for which the new converger is to be used.
         */
        NullConverger(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs a NullConverger.
        /**
         * \param copy The converger from which properties are to be duplicated
         *             into this.
         */
        NullConverger(
            const NullConverger& copy
            );

        /**
         * \brief Copy constructs a NullConverger for use by \a algorithm.
         *
         * \param copy The converger from which properties are to be duplicated
         *             into this.
         * \param algorithm The GA for which the new converger is to be used.
         */
        NullConverger(
            const NullConverger& copy,
            GeneticAlgorithm& algorithm
            );

}; // class NullConverger


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
#include "./inline/NullConverger.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_NULLCONVERGER_HPP
