/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class RegionOfSpace.

    NOTES:

        See notes of RegionOfSpace.hpp.

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
 * \brief Contains the inline methods of the RegionOfSpace class.
 */




/*
================================================================================
Includes
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
Inline Mutators
================================================================================
*/

inline
void
RegionOfSpace::SetLowerLimit(
    eddy::utilities::DoubleExtremes::size_type dim,
    double value
    )
{
    _limits.set_min(dim, value);
}

inline
void
RegionOfSpace::SetUpperLimit(
    eddy::utilities::DoubleExtremes::size_type dim,
    double value
    )
{
    _limits.set_max(dim, value);
}

inline
void
RegionOfSpace::SetLimits(
    eddy::utilities::DoubleExtremes::size_type dim,
    double lowerLimit,
    double upperLimit
    )
{
    _limits.set_min(dim, lowerLimit);
    _limits.set_max(dim, upperLimit);
}







/*
================================================================================
Inline Accessors
================================================================================
*/








/*
================================================================================
Inline Public Methods
================================================================================
*/
inline
eddy::utilities::DoubleExtremes::size_type
RegionOfSpace::Dimensionality(
    ) const
{
    return _limits.size();
}

inline
JEGA::DoubleVector
RegionOfSpace::GetLowerLimits(
    ) const
{
    return _limits.get_mins();
}

inline
JEGA::DoubleVector
RegionOfSpace::GetUpperLimits(
    ) const
{
    return _limits.get_maxs();
}

inline
double
RegionOfSpace::GetLowerLimit(
    eddy::utilities::DoubleExtremes::size_type dim
    ) const
{
    return _limits.get_min(dim);
}

inline
double
RegionOfSpace::GetUpperLimit(
    eddy::utilities::DoubleExtremes::size_type dim
    ) const
{
    return _limits.get_max(dim);
}








/*
================================================================================
Inline Subclass Visible Methods
================================================================================
*/








/*
================================================================================
Inline Private Methods
================================================================================
*/








/*
================================================================================
Inline Structors
================================================================================
*/








/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA

