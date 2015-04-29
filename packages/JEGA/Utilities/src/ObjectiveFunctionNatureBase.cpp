/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class ObjectiveFunctionNatureBase.

    NOTES:

        See notes of ObjectiveFunctionNatureBase.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Sun Sep 14 13:59:56 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the ObjectiveFunctionNatureBase class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ObjectiveFunctionTypeBase.hpp>
#include <../Utilities/include/ObjectiveFunctionNatureBase.hpp>


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
ObjectiveFunctionNatureBase::GetDesignTarget(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().GetDesignTarget();

} // ObjectiveFunctionNatureBase::GetDesignTarget

DesignTarget&
ObjectiveFunctionNatureBase::GetDesignTarget(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().GetDesignTarget();

} // ObjectiveFunctionNatureBase::GetDesignTarget

ObjectiveFunctionInfo&
ObjectiveFunctionNatureBase::GetObjectiveFunctionInfo(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().GetObjectiveFunctionInfo();

} // ObjectiveFunctionNatureBase::GetObjectiveFunctionInfo

const ObjectiveFunctionInfo&
ObjectiveFunctionNatureBase::GetObjectiveFunctionInfo(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().GetObjectiveFunctionInfo();

} // ObjectiveFunctionNatureBase::GetObjectiveFunctionInfo





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


ObjectiveFunctionNatureBase::ObjectiveFunctionNatureBase(
    ObjectiveFunctionTypeBase& type
    ) :
        _type(type)
{
    EDDY_FUNC_DEBUGSCOPE

} // ObjectiveFunctionNatureBase::ObjectiveFunctionNatureBase

ObjectiveFunctionNatureBase::ObjectiveFunctionNatureBase(
    const ObjectiveFunctionNatureBase&,
    ObjectiveFunctionTypeBase& type
    ) :
        _type(type)
{
    EDDY_FUNC_DEBUGSCOPE


} // ObjectiveFunctionNatureBase::ObjectiveFunctionNatureBase

ObjectiveFunctionNatureBase::~ObjectiveFunctionNatureBase(
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

