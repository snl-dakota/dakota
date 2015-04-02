/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class MOGA.

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

        Mon Jun 02 14:26:59 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the MOGA class.
 */





/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_MOGA_HPP
#define JEGA_ALGORITHMS_MOGA_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithm.hpp>









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
class MOGA;





/*
================================================================================
Class Definition
================================================================================
*/

/// The class that performs a Multi-Objective Genetic Algorithm
/**
 * This derived type of GeneticAlgorithm is specialized for multi-
 * objective problems.  It seeks the entire set of Pareto-dominant
 * solutions.
 *
 * This class differs from the GeneticAlgorithm class by only a small degree.
 *
 * The MOGA overrides a few virtual methods of the GeneticAlgorithm base class.
 * Other than that, the only difference is in its operator group registry.
 */
class JEGA_SL_IEDECL MOGA :
    public GeneticAlgorithm
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /**
         * \brief This flag indicates whether or not this class properly
         *        registered the appropriate operator groups on startup.
         */
        static const bool _registered_operator_groups;

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

        /**
         * \brief This static method returns the registry of operator
         *        groups available to MOGA's.
         *
         * \return All operator groups that can be used when using a MOGA.
         */
        static
        GeneticAlgorithmOperatorGroupRegistry&
        RegistryOfOperatorGroups(
            );

        /**
         * \brief Considers each discarded Design in the target and takes it
         *        back if it is non-dominated by the current population.
         *
         * This may introduce designs into the population that dominate
         * existing members.  If that is the case and you want a purely non-
         * dominated population, then you should take an additional step to
         * remove dominated solutions.
         */
        virtual
        void
        ReclaimOptimal(
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

        /// Gets one best design in the current population.
        /**
         * This is a very subjective matter especially for a MOGA.  This method
         * is only here to support the needs of DAKOTA and should be avoided
         * in general.
         *
         * The current implementation finds the point closest in distance to
         * the currently known utopia point.
         *
         * \return The single best Design currently in the population.
         */
        virtual
        const JEGA::Utilities::Design*
        GetBestDesign(
            );

        /// Returns the current set of solutions of this algorithm.
        /**
         * This returns the set of all Pareto optimal solutions taken from
         * the current population and the discards.  If any designs are
         * cached by any operators, they will not be included.  If no feasible
         * designs exist, then the return is an empty set.
         *
         * The solutions in the returned set are created using the new operator
         * but are not "kept track of".  It will be up to the user to
         * explicitly call "flush" on the returned set when you are done
         * with it.  This will cause destruction of all the created solutions.
         * If this does not happen, there will be a memory leak.
         *
         * \return The set of solutions currently found by this algorithm
         */
        virtual
        JEGA::Utilities::DesignOFSortSet
        GetCurrentSolution(
            ) const;

        /**
         * \brief This method returns the registry of all allowed groups for
         *        use with a MOGA.
         *
         * These groups define what operators can be used with this algorithm
         * and in what configurations (i.e. what operators can be used with
         * what other operators).
         *
         * \return The registry of all operator groups that can be used with
         *         a MOGA.
         */
        virtual
        GeneticAlgorithmOperatorGroupRegistry&
        GetOperatorGroupRegistry(
            );

        /**
         * \brief Overriden to look through the current designs and remove
         *        any that are not optimal.
         *
         * Removeed designs are placed into the targets discards.
         */
        virtual
        void
        FlushNonOptimal(
            );

        /**
         * \brief Return the name of the type of this GA.
         *
         * \return The string "moga".
         */
        virtual
        std::string
        GetAlgorithmTypeName(
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

        /// Constructs a MOGA which will use \a target.
        /**
         * \param target The design target to be used by this algorithm.
         * \param log The log into which this and all its operators will log.
         */
        MOGA(
            JEGA::Utilities::DesignTarget& target,
            JEGA::Logging::Logger& log
            );


}; // class MOGA


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
#include "./inline/MOGA.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_MOGA_HPP
