/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class InfoBase.

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
 * \brief Contains the implementation of the InfoBase class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/InfoBase.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>





/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;







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








/*
================================================================================
Subclass Visible Methods
================================================================================
*/








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
InfoBase::InfoBase(
    DesignTarget& target
    ) :
        _target(target),
        _number(0)
{
    EDDY_FUNC_DEBUGSCOPE
}

InfoBase::InfoBase(
    const InfoBase& copy
    ) :
        _target(copy._target),
        _label(copy._label),
        _number(copy._number)
{
    EDDY_FUNC_DEBUGSCOPE
}

InfoBase::InfoBase(
    const InfoBase& copy,
    DesignTarget& target
    ) :
        _target(target),
        _label(copy._label),
        _number(copy._number)
{
    EDDY_FUNC_DEBUGSCOPE
}

InfoBase::~InfoBase(
    )
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
