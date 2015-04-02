/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class GeneticAlgorithmSelector

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

        Wed May 21 15:26:36 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the GeneticAlgorithmSelector class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_GENETICALGORITHMSELECTOR_HPP
#define JEGA_ALGORITHMS_GENETICALGORITHMSELECTOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utility>
#include <GeneticAlgorithmOperator.hpp>
#include <utilities/include/int_types.hpp>
#include <../Utilities/include/DesignMultiSet.hpp>








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
        class DesignGroup;
        class DesignOFSortSet;
        class DesignGroupVector;
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
class GeneticAlgorithmSelector;




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

/// The base class for all GeneticAlgorithm selectors.
/**
 * The selectors is responsible for choosing the next population from the
 * groups of designs passed into it.  Chosen designs should be removed
 * from the groups they are taken from and inserted into the group passed
 * in to recieve them.
 *
 * It is not necessary to do anything with the remaining Designs
 * in the passed in groups.  The GA will handle them.
 *
 * This base class requires of it's derivatives implementation of the
 * Select method.  It requires no configuration input beyond that required by
 * its base class.
 */
class JEGA_SL_IEDECL GeneticAlgorithmSelector :
    public GeneticAlgorithmOperator
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:


    protected:

        typedef
        std::pair<std::size_t, JEGA::Utilities::DesignOFSortSet::iterator>
        DesignGroupInfo;

    private:

    /*
    ============================================================================
    Nested Inner Classes
    ============================================================================
    */
    public:

        /// This predicate can be used to sort by Fitness value.
        class FitnessPred
        {
            /*
            ====================================================================
            Member Data Declarations
            ====================================================================
            */
            private:

                /// The fitnesses that this predicate will use to sort by.
                const FitnessRecord& _ftns;

            /*
            ====================================================================
            Public Methods
            ====================================================================
            */
            public:

                /// One of the operators that activates this predicate
                /**
                 * \param des1 The first design to compare by fitness value.
                 * \param des2 The second design to compare by fitness value.
                 * \return True if the fitness value for \a des1 is greater than
                 *         that for \a des2 and false otherwise.
                 */
                inline
                bool
                operator ()(
                    const JEGA::Utilities::Design* des1,
                    const JEGA::Utilities::Design* des2
                    ) const;

                /// One of the operators that activates this predicate
                /**
                 * \param d1Info The first design with group info to compare by
                 *               fitness value.
                 * \param d2Info The second design with group info to compare by
                 *               fitness value.
                 * \return True if the fitness value for the design in \a d1Info
                 *         is greater than that for \a d2Info and false
                 *           otherwise.
                 */
                inline
                bool
                operator ()(
                    const DesignGroupInfo& d1Info,
                    const DesignGroupInfo& d2Info
                    ) const;

            /*
            ====================================================================
            Structors
            ====================================================================
            */
            public:

                /**
                 * \brief Constructs a FitnessPred to sort according to the
                 *        supplied fitnesses.
                 *
                 * \param ftns The record of fitnesses from which to get the
                 *             fitness values of the designs to compare.
                 */
                inline
                FitnessPred(
                    const FitnessRecord& ftns
                    );

        }; // class FitnessPred

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

        virtual
        JEGA::Utilities::DesignOFSortSet
        SelectNBest(
            JEGA::Utilities::DesignGroupVector& from,
            std::size_t n,
            const FitnessRecord& ftns
            );



    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:

        /// Simply erases the supplied Design from any and all supplied groups.
        /**
         * This does not send the Design to the target or anything else.  It
         * just takes it out of the groups.  If you have not accounted for it
         * elsewhere, there is a risk that it will be lost and that there will
         * be a memory leak.
         *
         * \param design The Design to remove from any and all groups in
         *               \a groups.
         * \param groups The groups from which \a design is to be removed.
         */
        static
        void
        RemoveFromGroups(
            const JEGA::Utilities::Design* design,
            const JEGA::Utilities::DesignGroupVector& groups
            );

        /**
         * \brief Chooses the \a n best designs in from and places them into
         *        \a into.
         *
         * "Best" means those with the highest fitness values as indicated in
         * \a fitnesses.
         *
         * \param from The groups from which to select the best designs.
         * \param into The group into which to place those designs that get
         *             selected.
         * \param n The number of designs to choose.
         * \param fitnesses The record of fitnesses for all of the designs in
         *                  \a from.
         * \param comp The predicate to use in sorting designs in improving
         *               order such that better is earlier in the list.  i.e. The
         *             best design will be first and the worst last.
         */
        template <typename CompT>
        static
        void
        SelectNBest(
            JEGA::Utilities::DesignGroupVector& from,
            JEGA::Utilities::DesignGroup& into,
            std::size_t n,
            const FitnessRecord& fitnesses,
            CompT comp
            );

        template <typename CompT>
        static
        JEGA::Utilities::DesignOFSortSet
        SelectNBest(
            JEGA::Utilities::DesignGroupVector& from,
            std::size_t n,
            CompT comp
            );


    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /// This is the method in which derived selectors do whatever they do.
        /**
         * It is called by the genetic algorithm periodically.
         * Required information can be retrieved through the
         * GeneticAlgorithmOperator::GetAlgorithm() base class method.
         *
         * Selections should be made from the groups in \a from and placed into
         * \a into. \a count is the requested number of selections but can be
         * ignored if you wish.
         *
         * If you choose to allow the same Design to be selected into the next
         * population multiple times, be certain to copy construct it using the
         * GeneticAlgorithm::GetNewDesign(Design*) method.
         * DO NOT PUT THE EXACT SAME DESIGN INTO THE \a into GROUP MORE THAN
         * ONCE.
         *
         * \param from The collection of groups from which to select designs.
         * \param into The single group into which to place selected designs.
         * \param count The number of Designs that the algorithm would like to
         *              have selected.  Just a recommendation.
         * \param fitnesses The record of fitness values assigned to each of
         *                  the designs in \a from.
         */
        virtual
        void
        Select(
            JEGA::Utilities::DesignGroupVector& from,
            JEGA::Utilities::DesignGroup& into,
            std::size_t count,
            const FitnessRecord& fitnesses
            ) = 0;


        /// Retrieves specific parameters using Get...FromDB methods.
        /**
         * This method is used to extract needed information for this
         * operator.  It does so using the "Get...FromDB" class
         * of methods from the GeneticAlgorithmOperator base class.
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
         * \return The string "Selector".
         */
        virtual
        std::string
        GetType(
            ) const;

        virtual
        bool
        CanSelectSameDesignMoreThanOnce(
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

        /// Constructs a GeneticAlgorithmSelector for use by \a algorithm.
        /**
         * \param algorithm The GA for which this selector is being
         *                  constructed.
         */
        GeneticAlgorithmSelector(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs a GeneticAlgorithmSelector.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        GeneticAlgorithmSelector(
            const GeneticAlgorithmSelector& copy
            );

        /// Copy constructs a GeneticAlgorithmSelector for use by \a algorithm.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this selector is being
         *                  constructed.
         */
        GeneticAlgorithmSelector(
            const GeneticAlgorithmSelector& copy,
            GeneticAlgorithm& algorithm
            );



}; // class GeneticAlgorithmSelector


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
#include "./inline/GeneticAlgorithmSelector.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_GENETICALGORITHMSELECTOR_HPP
