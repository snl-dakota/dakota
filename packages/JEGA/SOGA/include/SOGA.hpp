/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class SOGA.

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
 * \brief Contains the definition of the SOGA class.
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_SOGA_HPP
#define JEGA_ALGORITHMS_SOGA_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <memory> // for auto_ptr
#include <GeneticAlgorithm.hpp>
#include <../Utilities/include/JEGATypes.hpp>









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
        class SolutionOFSortSet;
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
In Namespace File Scope Typedefs
================================================================================
*/







/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
class SOGA;





/*
================================================================================
Class Definition
================================================================================
*/

/// The class that performs a Single-Objective Genetic Algorithm
/**
 * This derived type of GeneticAlgorithm is specialized for single
 * objective problems.  It seeks the one best solution.
 *
 * This class differs from the GeneticAlgorithm class by only a small degree.
 *
 * The SOGA overrides a few virtual methods of the GeneticAlgorithm base class.
 * Other than that, the only difference is in its operator group registry.
 */
class JEGA_SL_IEDECL SOGA :
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
         * registered the appropriate operator groups on startup.
         */
        static const bool _registered_operator_groups;

        /// This is the vector of weights to apply to the objective functions
        JEGA::DoubleVector _weights;

    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


        /// Sets the weights for this algorithm to use.
        /**
         * \param weights The weights used by this algorithm to create weighted
         *                sums of objectives.
         */
        void
        SetWeights(
            const JEGA::DoubleVector& weights
            );




    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:


        /// Returns the set of weights for this algorithm by const_reference
        /**
         * \return The weights currently being used by this algorithm to create
         *         weighted sums of objectives.
         */
        inline
        const JEGA::DoubleVector&
        GetWeights(
            ) const;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /**
         * \brief This static method returns the registry of operator
         *        groups available to SOGA's.
         *
         * \return All operator groups that can be used when using a SOGA.
         */
        static
        GeneticAlgorithmOperatorGroupRegistry&
        RegistryOfOperatorGroups(
            );

        /**
         * \brief Considers each discarded Design in the target and takes it
         *        back if it is optimal compared to the current population.
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

        /**
         * \brief Retrieves specific parameters for this algorithm.
         *
         * This method is used to extract needed information for this
         * algorithm.  It does so using the ParameterExtractor class
         * methods. The return value is true if the extraction
         * completes successfully and false otherwise.
         *
         * \param db The database of parameter values from which to do
         *           retrieval.
         * \return true if polling completed successfully and false otherwise.
         */
        virtual
        bool
        PollForParameters(
            const JEGA::Utilities::ParameterDatabase& db
            );

        /// Returns the current set of solutions of this algorithm.
        /**
         * This returns the set of all Solutions that have the same quality
         * as defined by their weighted sums.  Only feasible designs are
         * considered.  If no feasible designs exist, then the return is
         * an empty set. If any designs are  cached by any operators, they will
         * not be included.
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
         *        use with a SOGA..
         *
         * These groups define what operators can be used with this algorithm
         * and in what configurations (i.e. what operators can be used with
         * what other operators).
         *
         * \return The registry of all operator groups that can be used with
         *         a SOGA..
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
         * \return The string "soga".
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

        /// Constructs a SOGA which will use \a target.
        /**
         * \param target The design target to be used by this algorithm.
         * \param log The log into which this and all its operators will log.
         */
        SOGA(
            JEGA::Utilities::DesignTarget& target,
            JEGA::Logging::Logger& log
            );


}; // class SOGA


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
#include "./inline/SOGA.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_SOGA_HPP
