/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class ConstraintNatureBase.

    NOTES:

        See notes of ConstraintNatureBase.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jun 10 10:23:36 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the ConstraintNatureBase class.
 */





/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ConstraintTypeBase.hpp>
#include <../Utilities/include/ConstraintNatureBase.hpp>




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

const DesignTarget&
ConstraintNatureBase::GetDesignTarget(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().GetDesignTarget();

} // ConstraintNatureBase::GetDesignTarget

DesignTarget&
ConstraintNatureBase::GetDesignTarget(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().GetDesignTarget();

} // ConstraintNatureBase::GetDesignTarget

ConstraintInfo&
ConstraintNatureBase::GetConstraintInfo(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().GetConstraintInfo();

} // ConstraintNatureBase::GetConstraintInfo

const ConstraintInfo&
ConstraintNatureBase::GetConstraintInfo(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().GetConstraintInfo();

} // ConstraintNatureBase::GetConstraintInfo







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
ConstraintNatureBase::ConstraintNatureBase(
    ConstraintTypeBase& type
    ) :
        _type(type)
{
    EDDY_FUNC_DEBUGSCOPE

} // ConstraintNatureBase::ConstraintNatureBase

ConstraintNatureBase::ConstraintNatureBase(
    const ConstraintNatureBase&,
    ConstraintTypeBase& type
    ) :
        _type(type)
{
    EDDY_FUNC_DEBUGSCOPE


} // ConstraintNatureBase::ConstraintNatureBase

ConstraintNatureBase::~ConstraintNatureBase(
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
