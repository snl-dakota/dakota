/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class FavorFeasibleSelector.

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

        Wed Jul 23 10:32:44 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the FavorFeasibleSelector class.
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_FAVORFEASIBLESELECTOR_HPP
#define JEGA_ALGORITHMS_FAVORFEASIBLESELECTOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmSelector.hpp>





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
class FavorFeasibleSelector;






/*
================================================================================
Class Definition
================================================================================
*/

/// This selector favors feasible over infeasible and then goes by fitness.
/**
 * This selector will always take a feasible Design over an infeasible one.
 * Beyond that, it favors Designs based on assigned fitness values.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class FavorFeasibleSelector:
    public GeneticAlgorithmSelector
{
    /*
    ============================================================================
    Nested Inner Class Forward Declares
    ============================================================================
    */
    private:

        class FeasibilityPred;

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


        ///Returns the proper name of this operator
        /**
         * \return The string "favor_feasible".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
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
         * \param algorithm The GA for which the new selector is to be used.
         * \return A new, default instance of a FavorFeasibleSelector.
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

        ///Performs SOGA selection
        virtual
        void
        Select(
            JEGA::Utilities::DesignGroupVector& from,
            JEGA::Utilities::DesignGroup& into,
            std::size_t count,
            const FitnessRecord& fitnesses
            );

        virtual
        JEGA::Utilities::DesignOFSortSet
        SelectNBest(
            JEGA::Utilities::DesignGroupVector& from,
            std::size_t n,
            const FitnessRecord& ftns
            );

        // Returns the proper name of this operator.
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
        CanSelectSameDesignMoreThanOnce(
            ) const;

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

        /// Constructs a FavorFeasibleSelector for use by \a algorithm.
        FavorFeasibleSelector(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs a FavorFeasibleSelector.
        FavorFeasibleSelector(
            const FavorFeasibleSelector& copy
            );

        /// Copy constructs a FavorFeasibleSelector for use by \a algorithm.
        FavorFeasibleSelector(
            const FavorFeasibleSelector& copy,
            GeneticAlgorithm& algorithm
            );




}; // class FavorFeasibleSelector


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
#include "./inline/FavorFeasibleSelector.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_FAVORFEASIBLESELECTOR_HPP
