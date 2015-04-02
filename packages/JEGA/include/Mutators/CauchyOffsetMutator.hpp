/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class CauchyOffsetMutator.

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

        Thu Jul 10 09:23:33 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the CauchyOffsetMutator class.
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_CAUCHYOFFSETMUTATOR_HPP
#define JEGA_ALGORITHMS_CAUCHYOFFSETMUTATOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <Mutators/OffsetMutatorBase.hpp>








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
class CauchyOffsetMutator;







/*
================================================================================
Class Definition
================================================================================
*/

/// Mutates by adding Cauchy noise to variable values.
/**
 * This mutator introduces random variation by adding a Cauchy random
 * amount to a variable value.  The random amount has a median of 0 and a
 * standard deviation dependent on the offset range.
 *
 * For this offset mutator, the offset range is interpreted as a
 * fraction of the total range of the variable.  The standard deviation
 * is computed as the product of the offset range and the total range of
 * the variable.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class CauchyOffsetMutator :
    public OffsetMutatorBase
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
         * \return The string "offset_cauchy".
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
            It then chooses a random design variable and adds a Cauchy random
            amount to it.  The amount has a mean of 0 and a standard deviation
            that depends on the offset range.  The offset range is interpreted
            as a precentage of the total range of the variable and that amount
            is in turn interpreted as the standard deviation.  The number of
            mutations is the rate times the size of the group passed in rounded
            to the nearest whole number.
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
         * \return A new, default instance of a CauchyOffsetMutator.
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
         * \brief This method provides an appropriate offset amount for the
         *        requested variable using a gaussian random distribution.
         *
         * \param varInfo The information object describing the design variable
         *                currently being considered for mutation and for which
         *                an offset amount is desired.
         * \return The amount by which to offset a value for the variable
         *         described by \a varInfo.
         */
        virtual
        double
        GetOffsetAmount(
            const JEGA::Utilities::DesignVariableInfo& varInfo
            ) const;

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


        /// Constructs an CauchyOffsetMutator for use by \a algorithm.
        /**
         * \param algorithm The GA for which this mutator is being constructed.
         */
        CauchyOffsetMutator(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an CauchyOffsetMutator.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        CauchyOffsetMutator(
            const CauchyOffsetMutator& copy
            );

        /**
         * \brief Copy constructs an CauchyOffsetMutator for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this mutator is being constructed.
         */
        CauchyOffsetMutator(
            const CauchyOffsetMutator& copy,
            GeneticAlgorithm& algorithm
            );

}; // class CauchyOffsetMutator


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
#include "./inline/CauchyOffsetMutator.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_CAUCHYOFFSETMUTATOR_HPP
