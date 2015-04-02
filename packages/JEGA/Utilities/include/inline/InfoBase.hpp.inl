/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class InfoBase.

    NOTES:

        See notes of InfoBase.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Jun 09 16:33:08 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the InfoBase class.
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
InfoBase::SetLabel(
    const std::string& text
    )
{
    _label.assign(text);
}

inline
void
InfoBase::SetNumber(
    std::size_t number
    )
{
    _number = number;
}









/*
================================================================================
Inline Accessors
================================================================================
*/

inline
const std::string&
InfoBase::GetLabel(
    ) const
{
    return _label;
}

inline
std::size_t
InfoBase::GetNumber(
    ) const
{
    return _number;
}


inline
DesignTarget&
InfoBase::GetDesignTarget(
    )
{
    return _target;
}

inline
const DesignTarget&
InfoBase::GetDesignTarget(
    ) const
{
    return _target;
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
