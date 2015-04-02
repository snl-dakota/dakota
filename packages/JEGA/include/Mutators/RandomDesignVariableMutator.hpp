/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class RandomDesignVariableMutator.

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

        Wed Jun 04 12:07:44 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the RandomDesignVariableMutator class.
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_RANDOMDESIGNVARIABLEMUTATOR_HPP
#define JEGA_ALGORITHMS_RANDOMDESIGNVARIABLEMUTATOR_HPP







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
class RandomDesignVariableMutator;







/*
================================================================================
Class Definition
================================================================================
*/


/// Mutates by random design variable reassignment.
/**
 * This mutator introduces random variation by first randomly choosing a design
 * variable of a randomly selected design and reassigning it to a random valid
 * value for that variable.  No consideration of the current value is given
 * when determining the new value.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class RandomDesignVariableMutator :
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
         * \return The string "replace_uniform".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This mutator does mutation by first randomly selecting a Design.
            It then chooses a random design variable and reassigns that
            variable to a random valid value.

            The number of mutations is the rate times the size of the group
            passed in rounded to the nearest whole number.
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
         * \return A new, default instance of a RandomDesignVariableMutator.
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

        /// Performs random design variable mutation.
        /**
         * This mutator only mutates members of the population but places the
         * resulting mutated designs into the children.
         *
         * \param pop The population of designs to which to apply mutation.
         * \param cldrn The group of child designs to which to apply mutation.
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

        /// Constructs a RandomDesignVariableMutator for use by \a algorithm.
        /**
         * \param algorithm The GA for which this mutator is being constructed.
         */
        RandomDesignVariableMutator(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs a RandomDesignVariableMutator.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        RandomDesignVariableMutator(
            const RandomDesignVariableMutator& copy
            );

        /**
         * \brief Copy constructs a RandomDesignVariableMutator for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this mutator is being constructed.
         */
        RandomDesignVariableMutator(
            const RandomDesignVariableMutator& copy,
            GeneticAlgorithm& algorithm
            );


}; // class RandomDesignVariableMutator


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
#include "./inline/RandomDesignVariableMutator.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_RANDOMDESIGNVARIABLEMUTATOR_HPP
