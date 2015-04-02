/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class ObjectiveFunctionTypeBase.

    NOTES:

        See notes of ObjectiveFunctionTypeBase.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Sep 08 12:24:09 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the ObjectiveFunctionTypeBase class.
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
ObjectiveFunctionInfo&
ObjectiveFunctionTypeBase::GetObjectiveFunctionInfo(
    )
{
    return _info;
}

inline
const ObjectiveFunctionInfo&
ObjectiveFunctionTypeBase::GetObjectiveFunctionInfo(
    ) const
{
    return _info;
}

inline
ObjectiveFunctionNatureBase&
ObjectiveFunctionTypeBase::GetNature(
    ) const
{
    return *_nature;
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

