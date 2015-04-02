/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class RegionOfSpace.

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

        Thu Apr 13 07:42:48 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the RegionOfSpace class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_REGIONOFSPACE_HPP
#define JEGA_UTILITIES_REGIONOFSPACE_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <iosfwd>
#include <utilities/include/extremes.hpp>
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
class DesignTarget;
class RegionOfSpace;







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
 * \brief
 *
 *
 */
class RegionOfSpace
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

        eddy::utilities::DoubleExtremes _limits;


    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        inline
        void
        SetLowerLimit(
            eddy::utilities::DoubleExtremes::size_type dim,
            double value
            );

        inline
        void
        SetUpperLimit(
            eddy::utilities::DoubleExtremes::size_type dim,
            double value
            );

        inline
        void
        SetLimits(
            eddy::utilities::DoubleExtremes::size_type dim,
            double lowerLimit,
            double upperLimit
            );

    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Returns the vector of lower limit values for this region.
        /**
         * \return The lower bounds of all dimensions of this space region.
         */
        inline
        JEGA::DoubleVector
        GetLowerLimits(
            ) const;

        /// Returns the vector of upper limit values for this region.
        /**
         * \return The upper bounds of all dimensions of this space region.
         */
        inline
        JEGA::DoubleVector
        GetUpperLimits(
            ) const;

        /**
         * \brief Returns the lower limit value for the supplied dimension of
         *        this region.
         *
         * \return The lower bound of the \a dim dimension of this space
         *         region.
         */
        inline
        double
        GetLowerLimit(
            eddy::utilities::DoubleExtremes::size_type dim
            ) const;

        /**
         * \brief Returns the upper limit value for the supplied dimension of
         *        this region.
         *
         * \return The upper bound of the \a dim dimension of this space
         *         region.
         */
        inline
        double
        GetUpperLimit(
            eddy::utilities::DoubleExtremes::size_type dim
            ) const;

    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        inline
        eddy::utilities::DoubleExtremes::size_type
        Dimensionality(
            ) const;

        bool
        Intersects(
            const RegionOfSpace& other
            ) const;

        bool
        Intersects(
            eddy::utilities::DoubleExtremes::size_type dim,
            double lowerLimit,
            double upperLimit
            ) const;

        RegionOfSpace
        Intersection(
            const RegionOfSpace& other
            );

        bool
        Contains(
            const RegionOfSpace& other
            ) const;

        void
        Clear(
            );

        bool
        IsEmpty(
            ) const;

        double
        Volume(
            ) const;

        std::ostream&
        StreamOut(
            std::ostream& stream
            ) const;

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

        bool
        IsValid(
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

        RegionOfSpace(
            eddy::utilities::DoubleExtremes::size_type nDim
            );


        RegionOfSpace(
            const RegionOfSpace& copy
            );


}; // class RegionOfSpace



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
#include "inline/RegionOfSpace.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_REGIONOFSPACE_HPP
