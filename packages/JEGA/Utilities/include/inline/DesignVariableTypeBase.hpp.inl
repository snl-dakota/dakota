/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class DesignVariableTypeBase.

    NOTES:

        See notes of DesignVariableTypeBase.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jun 03 08:55:03 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the DesignVariableTypeBase class.
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
DesignVariableInfo&
DesignVariableTypeBase::GetDesignVariableInfo(
    )
{
    return _info;
}

inline
const DesignVariableInfo&
DesignVariableTypeBase::GetDesignVariableInfo(
    ) const
{
    return _info;
}

inline
DesignVariableNatureBase&
DesignVariableTypeBase::GetNature(
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
