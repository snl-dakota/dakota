/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class SingleObjectiveStatistician.

    NOTES:

        See notes under section Class Definition of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 22 08:02:43 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the SingleObjectiveStatistician class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_SINGLEOBJECTIVESTATISTICIAN_HPP
#define JEGA_UTILITIES_SINGLEOBJECTIVESTATISTICIAN_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <limits>
#include <../Utilities/include/JEGATypes.hpp>







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
    namespace Utilities {







/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
class Design;
class DesignGroup;
class WeightedSumMap;
class DesignOFSortSet;
class DesignGroupVector;
class SingleObjectiveStatistician;





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
/// A classe that computes single-objective statistics about DesignGroups.
/**
 * This class has the capabilities to compute various single-objective
 * statistics about collections of designs.  Examples are:
 *
 *   Computing weighted sum values using specified weights.
 *   Finding Designs associated with maximal or minimal weighted sums etc.
 */
class SingleObjectiveStatistician
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:


    protected:


    private:


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

        /**
         * \brief Computes the weighted sum for \a des and returns it.
         *
         * \param des The design whose weighted sum is of interest.
         * \param weights The set of weights to apply to the individual
         *                objectives.
         * \return the resulting weighted sum.
         */
        static
        double
        ComputeWeightedSum(
            const Design& des,
            const JEGA::DoubleVector& weights
            );

        /**
         * \brief Returns a map containing all the Designs in \a weights paired
         *        with their weighted sum.
         *
         * DesignContainer must hold Design*'s and be forward iteratable in STL
         * style.  It must also support the begin and end methods.
         *
         * \param designs The designs to compute the weighted sums of.
         * \param weights The set of weights to apply to the individual
         *                objectives.
         * \return A mapping of each Design to its weighted sum.
         */
        template <typename DesignContainer>
        static
        WeightedSumMap
        ComputeWeightedSums(
            const DesignContainer& designs,
            const JEGA::DoubleVector& weights
            );

        static
        WeightedSumMap
        ComputeWeightedSums(
            const DesignGroup& designs,
            const JEGA::DoubleVector& weights
            );

        static
        WeightedSumMap
        ComputeWeightedSums(
            const DesignGroupVector& designs,
            const JEGA::DoubleVector& weights
            );

        /**
         * \brief Finds the lowest weighted sum of any evaluated Design
         *        in the passed in container.
         *
         * DesignContainer must hold Design*'s and be forward iteratable in STL
         * style.
         *
         * \param cont The container of Designs to search.
         * \param weights The set of weights to apply to the individual
         *                objectives.
         * \return A mapping of the minimal weighted sum value to the locations
         *         of all Designs that have it in \a cont.
         */
        template <typename DesignContainer>
        static
        std::pair<
            double,
            std::vector<typename DesignContainer::const_iterator>
            >
        FindMinSumDesigns(
            const DesignContainer& cont,
            const JEGA::DoubleVector& weights
            );

        /**
         * \brief Finds the lowest weighted sum of any stored in the map.
         *
         * \param wsm The existing map of weighted sums to search.
         * \return A mapping of the minimal weighted sum value to all the
         *         Designs that have it in \a wsm.
         */
        static
        std::pair<double, std::vector<const Design*> >
        FindMinSumDesigns(
            const WeightedSumMap& wsm
            );

        /**
         * \brief Finds the lowest weighted sum of any feasible Design
         *        in the passed in cont.
         *
         * \param cont The container of Designs to search.
         * \param weights The set of weights to apply to the individual
         *                objectives.
         * \return A mapping of the minimal weighted sum value to the locations
         *         of all Designs that have it in \a cont considering only
         *         those that are feasible.
         */
        template <typename DesignContainer>
        static
        std::pair<
            double,
            std::vector<typename DesignContainer::const_iterator>
            >
        FindMinSumFeasibleDesigns(
            const DesignContainer& cont,
            const JEGA::DoubleVector& weights
            );

        /**
         * \brief Finds the Designs in the container that first are minimally
         *        violate of the constraints and second minimum in weighted sum.
         *
         * DesignContainer must hold Design*'s and be forward iteratable in STL
         * style.  If there are feasible designs in the solution set, then the
         * return of this method is the same as if having called
         * /a FindMinSumFeasibleDesigns.
         *
         * \param cont The container of Designs to search.
         * \param weights The set of weights to apply to the individual
         *                objectives.
         * \return A mapping of the minimal weighted sum value to the locations
         *         of all Designs that have it in \a cont and that are also
         *         closest to feasible.
         */
        template <typename DesignContainer>
        static
        std::pair<
            double,
            std::vector<typename DesignContainer::const_iterator>
            >
        FindMinSumMinViolateDesigns(
            const DesignContainer& cont,
            const JEGA::DoubleVector& weights
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
    private:

        /// Default constructs a SingleObjectiveStatistician.
        /**
         * This constructor is private and has no implementation.  This is
         * because all methods of this class are static and thus it
         * should not be instantiated.
         */
        SingleObjectiveStatistician(
            );


}; // class SingleObjectiveStatistician



/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA







/*
================================================================================
Include Inlined Methods File
================================================================================
*/
#include "./inline/SingleObjectiveStatistician.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_SINGLEOBJECTIVESTATISTICIAN_HPP
