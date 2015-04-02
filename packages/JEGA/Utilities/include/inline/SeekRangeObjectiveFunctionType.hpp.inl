/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class SeekRangeObjectiveFunctionType.

    NOTES:

        See notes of SeekRangeObjectiveFunctionType.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Sun Oct 12 17:36:14 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the SeekRangeObjectiveFunctionType
 *        class.
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
SeekRangeObjectiveFunctionType::SetLowerBound(
    double value
    )
{
    _lowerBound = value;
}

inline
void
SeekRangeObjectiveFunctionType::SetUpperBound(
    double value
    )
{
    _upperBound = value;
}





/*
================================================================================
Inline Accessors
================================================================================
*/

inline
double
SeekRangeObjectiveFunctionType::GetLowerBound(
    ) const
{
    return _lowerBound;
}

inline
double
SeekRangeObjectiveFunctionType::GetUpperBound(
    ) const
{
    return _upperBound;
}







/*
================================================================================
Inline Public Methods
================================================================================
*/








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

