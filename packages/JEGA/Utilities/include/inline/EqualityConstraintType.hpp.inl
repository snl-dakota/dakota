/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class EqualityConstraintType.

    NOTES:

        See notes of EqualityConstraintType.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jun 10 08:43:33 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the EqualityConstraintType class.
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
EqualityConstraintType::SetTargetValue(
    double val
    )
{
    _value = val;
}

inline
void
EqualityConstraintType::SetAllowableViolation(
    double viol
    )
{
    _viol = viol;
}





/*
================================================================================
Inline Accessors
================================================================================
*/

inline
double
EqualityConstraintType::GetTargetValue(
    ) const
{
    return _value;
}

inline
double
EqualityConstraintType::GetAllowableViolation(
    ) const
{
    return _viol;
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
