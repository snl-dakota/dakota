/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class LinearConstraintNature.

    NOTES:

        See notes of LinearConstraintNature.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jun 10 10:23:50 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the LinearConstraintNature class.
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








/*
================================================================================
Inline Accessors
================================================================================
*/

inline
const JEGA::DoubleVector&
LinearConstraintNature::GetCoefficients(
    ) const
{
    return _coeffs;
}

inline
JEGA::DoubleVector&
LinearConstraintNature::GetCoefficients(
    )
{
    return _coeffs;
}

inline
double
LinearConstraintNature::GetCoefficient(
    JEGA::DoubleVector::size_type elem
    ) const
{
    return _coeffs[elem];
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
