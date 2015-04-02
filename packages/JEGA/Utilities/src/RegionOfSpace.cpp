/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class RegionOfSpace.

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
 * \brief Contains the implementation of the RegionOfSpace class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utilities/include/Math.hpp>
#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/RegionOfSpace.hpp>







/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA;
using namespace JEGA::Logging;
using namespace eddy::utilities;







/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Utilities {





/*
================================================================================
Static Member Data Definitions
================================================================================
*/








/*
================================================================================
Mutators
================================================================================
*/








/*
================================================================================
Accessors
================================================================================
*/








/*
================================================================================
Public Methods
================================================================================
*/


bool
RegionOfSpace::Intersects(
    const RegionOfSpace& other
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(_limits.size() == other._limits.size())

    if(&other == this) return true;

    // An n-dimensional Euclidean retion intersects another if at least
    // one of it's corners lies within the other.  There are 2^n corners
    // where n is the dimensionality.  The Corners are defined as the set of
    // all possible extreme vectors such as all maxs, all mins, or any
    // combination.  It is actually easier to determine if it does not
    // intersect by comparing the maxs and mins.  If any of the maxs of
    // one of the spaces is less than the corresponding min of the other,
    // or visa versa, then they don't intersect.  Otherwise they do.
    for(DoubleVector::size_type i=0; i<_limits.size(); ++i)
        if(!Intersects(i, other._limits.get_min(i), other._limits.get_max(i)))
            return false;

    return true;
}

bool
RegionOfSpace::Intersects(
    DoubleExtremes::size_type dim,
    double lowerLimit,
    double upperLimit
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // There is no intersection if either upper limit is less than the other's
    // lower limit or visa versa.
    if(_limits.get_max(dim) <= lowerLimit) return false;
    if(upperLimit >= _limits.get_min(dim)) return false;

    // Otherwise we have intersection.
    return true;
}

bool
RegionOfSpace::Contains(
    const RegionOfSpace& other
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(_limits.size() == other._limits.size())

    if(&other == this) return true;

    // A region A is contained by another region B if all maxes of B are
    // greater than or equal to those of A and all mins of B are less than or
    // equal to those of A.
    for(DoubleVector::size_type i=0; i<_limits.size(); ++i)
    {
        if(_limits.get_max(i) < other._limits.get_max(i)) return false;
        if(_limits.get_min(i) > other._limits.get_min(i)) return false;
    }

    return true;
}

void
RegionOfSpace::Clear(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    _limits.clear(0.0, 0.0);
}

RegionOfSpace
RegionOfSpace::Intersection(
    const RegionOfSpace& other
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(_limits.size() == other._limits.size())

    if(&other == this) return other;

    // Create an initially empty region for return.
    RegionOfSpace ret(_limits.size());

    // If they don't intersect, just return the empty region.
    if(!Intersects(other)) return ret;

    // The intersection of two box-shaped regions, given that they are aligned
    // (neither is rotated from the euclidean axes) is another box-shaped
    // region.  To find it, for each dimension, take the lowest max and the
    // highest min.  Remember, we know they intersect.
    for(DoubleVector::size_type i=0; i<_limits.size(); ++i)
    {
        ret._limits.set_min(i,
            Math::Max(_limits.get_min(i), other._limits.get_min(i))
            );

        ret._limits.set_max(i,
            Math::Min(_limits.get_max(i), other._limits.get_max(i))
            );
    }

    EDDY_ASSERT(ret.IsValid());

    return ret;
}

bool
RegionOfSpace::IsEmpty(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    for(DoubleVector::size_type i=0; i<_limits.size(); ++i)
        if(_limits.get_max(i) == _limits.get_min(i)) return true;

    return false;
}

double
RegionOfSpace::Volume(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    double vol = 1.0;
    for(DoubleVector::size_type i=0; i<_limits.size(); ++i)
        vol *= (_limits.get_range(i));

    return vol;
}

ostream&
RegionOfSpace::StreamOut(
    ostream& stream
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // Write out the min max pairs after the index of each.
    for(DoubleExtremes::size_type i=0; i<_limits.size(); ++i)
        stream << i << ": " << _limits.get_min(i) << ", "
               << _limits.get_max(i) << '\n';

    return stream;
}


/*
================================================================================
Subclass Visible Methods
================================================================================
*/

bool
RegionOfSpace::IsValid(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    for(DoubleVector::size_type i=0; i<_limits.size(); ++i)
        if(_limits.get_max(i) < _limits.get_min(i)) return false;

    return true;
}







/*
================================================================================
Subclass Overridable Methods
================================================================================
*/








/*
================================================================================
Private Methods
================================================================================
*/








/*
================================================================================
Structors
================================================================================
*/

RegionOfSpace::RegionOfSpace(
    DoubleExtremes::size_type nDim
    ) :
        _limits(nDim, 0.0, 0.0)
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(nDim > 0)

    JEGAIFLOG_CF_II_G_F(nDim == 0, this, text_entry(lfatal(),
        "Cannot create a 0 dimensional region of space.")
        )
}

RegionOfSpace::RegionOfSpace(
    const RegionOfSpace& copy
    ) :
        _limits(copy._limits)
{
    EDDY_FUNC_DEBUGSCOPE
}






/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA

