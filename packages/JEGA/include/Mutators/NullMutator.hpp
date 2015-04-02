/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class NullMutator.

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

        Thu May 29 09:26:41 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the NullMutator class.
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_NULLMUTATOR_HPP
#define JEGA_ALGORITHMS_NULLMUTATOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmMutator.hpp>










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
class NullMutator;







/*
================================================================================
Class Definition
================================================================================
*/

/// The null object of mutators.
/**
 * This mutator does nothing.  It merely serves as a null object.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class NullMutator :
    public GeneticAlgorithmMutator
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
         * \return The string "null_mutation".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This mutator does nothing.  It is intended to serve as
            a null object for mutators.  Use it if you do not wish
            to perform any mutation.
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
         * \param algorithm The GA for which the new mutator is to be used.
         * \return A new, default instance of a NullMutator.
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
         * \brief This version of Mutate does absolutely nothing.
         *
         * \param pop The GA's population to which to apply mutation which
         *            here means nothing.
         * \param cldrn The GA's child designs to which to apply mutation which
         *              here means nothing.
         */
        virtual
        void
        Mutate(
            JEGA::Utilities::DesignGroup& pop,
            JEGA::Utilities::DesignGroup& cldrn
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

        virtual
        bool
        CanProduceInvalidVariableValues(
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

        /// Constructs an NullMutator for use by \a algorithm.
        /**
         * \param algorithm The GA for which this mutator is
         *                  being constructed.
         */
        NullMutator(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an NullMutator.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        NullMutator(
            const NullMutator& copy
            );

        /**
         * \brief Copy constructs an NullMutator for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this mutator is
         *                  being constructed.
         */
        NullMutator(
            const NullMutator& copy,
            GeneticAlgorithm& algorithm
            );

}; // class NullMutator


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
#include "./inline/NullMutator.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_NULLMUTATOR_HPP
