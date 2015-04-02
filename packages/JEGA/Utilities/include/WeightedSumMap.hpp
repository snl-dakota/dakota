/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class WeightedSumMap.

    NOTES:

        See notes under Class Definition section of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Tue Dec 20 12:33:44 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the WeightedSumMap class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_WEIGHTEDSUMMAP_HPP
#define JEGA_UTILITIES_WEIGHTEDSUMMAP_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/JEGATypes.hpp>
#include <../Utilities/include/DesignValueMap.hpp>







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
class WeightedSumMap;







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
/**
 * \brief A container class to map Designs to an associated weighted sum value.
 *
 * This is simply a DesignValueMap with the additional capability to store the
 * vector of weights used to compute the weighted sums.
 */
class JEGA_SL_IEDECL WeightedSumMap :
    public DesignDoubleMap
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

        /// The weights used to compute the weighted sums in this map.
        const JEGA::DoubleVector _weights;



    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /**
         * \brief Provides immutable access to the weights used in this map.
         *
         * \return The set of weights used to compute the weigthed sums present
         *         in this map.
         */
        inline
        const JEGA::DoubleVector&
        GetWeights(
            ) const;

    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /**
         * \brief Retrieves and returns the weighted sum associated with the
         *        supplied Design.
         *
         * \param of The Design whose weighted sum is sought.
         * \return The result of a call to DesignValueMap::GetValue.
         */
        inline
        double
        GetWeightedSum(
            const Design* of
            ) const;

        /**
         * \brief Retrieves and returns the weighted sum associated with the
         *        supplied Design.
         *
         * \param of The Design whose weighted sum is sought.
         * \return The result of a call to DesignValueMap::GetValue.
         */
        inline
        double
        GetWeightedSum(
            const Design& of
            ) const;

        /**
         * \brief Adds a mapping of Design "of" to the weighted sum "wsum".
         *
         * \param of The Design to map to the weighted sum "wsum".
         * \param wsum The weighted sum value to map to the Design "of".
         * \return The result of a call to DesignValueMap::AddValue.
         */
        inline
        bool
        AddWeightedSum(
            const Design& of,
            double wsum
            );

        /**
         * \brief Adds a mapping of Design "of" to the weighted sum "wsum".
         *
         * \param of The Design to map to the weighted sum "wsum".
         * \param wsum The weighted sum value to map to the Design "of".
         * \return The result of a call to DesignValueMap::AddValue.
         */
        inline
        bool
        AddWeightedSum(
            const Design* of,
            double wsum
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
    public:

        /**
         * \brief Constructs a weighted sum map in which weightes sums will
         *        exist that were computed using the supplied weights.
         *
         * \param weights The weights used to compute the weighted sums
         *                reported in this map.
         */
        inline
        WeightedSumMap(
            const JEGA::DoubleVector& weights,
            std::size_t initSize = 0
            );

        /**
         * \brief Copy constructs a WeightedSumMap
         *
         * \param copy The existing WeightedSumMap map to copy.
         */
        inline
        WeightedSumMap(
            const WeightedSumMap& copy
            );

        /**
         * \brief Destructs a WeightedSumMap.
         */
        virtual
        ~WeightedSumMap(
            ) {};



}; // class WeightedSumMap



/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
#include "inline/WeightedSumMap.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_WEIGHTEDSUMMAP_HPP
