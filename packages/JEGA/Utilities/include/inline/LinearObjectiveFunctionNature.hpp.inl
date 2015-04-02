/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class LinearObjectiveFunctionNature.

    NOTES:

        See notes of LinearObjectiveFunctionNature.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Sun Sep 14 16:40:20 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the LinearObjectiveFunctionNature
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








/*
================================================================================
Inline Accessors
================================================================================
*/


inline
const JEGA::DoubleVector&
LinearObjectiveFunctionNature::GetCoefficients(
    ) const
{
    return this->_coeffs;
}

inline
double
LinearObjectiveFunctionNature::GetCoefficient(
    JEGA::DoubleVector::size_type elem
    ) const
{
    return this->_coeffs[elem];
}

inline
JEGA::DoubleVector&
LinearObjectiveFunctionNature::GetCoefficients(
    )
{
    return this->_coeffs;
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

