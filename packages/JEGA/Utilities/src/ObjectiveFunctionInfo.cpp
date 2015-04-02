/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class ObjectiveFunctionInfo.

    NOTES:

        See notes of ObjectiveFunctionInfo.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Sep 08 12:19:58 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the ObjectiveFunctionInfo class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/Design.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ObjectiveFunctionInfo.hpp>
#include <../Utilities/include/ObjectiveFunctionTypeBase.hpp>
#include <../Utilities/include/MinimizeObjectiveFunctionType.hpp>


/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Utilities;







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


void
ObjectiveFunctionInfo::SetType(
    ObjectiveFunctionTypeBase* type
    )
{
    EDDY_FUNC_DEBUGSCOPE
    delete this->_type;
    this->_type = (type == 0x0) ?
        new MinimizeObjectiveFunctionType(*this) : type;

} // ObjectiveFunctionInfo::SetType






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
string
ObjectiveFunctionInfo::GetNatureString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().GetNatureString();

} // ObjectiveFunctionInfo::GetNatureString


bool
ObjectiveFunctionInfo::SetNature(
    ObjectiveFunctionNatureBase* nature
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().SetNature(nature);

} // ObjectiveFunctionInfo::SetNature


ObjectiveFunctionNatureBase&
ObjectiveFunctionInfo::GetNature(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().GetNature();

} // ObjectiveFunctionInfo::GetNature

int
ObjectiveFunctionInfo::PreferComp(
    double val1,
    double val2
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    val1 = this->GetType().GetValueForMinimization(val1);
    val2 = this->GetType().GetValueForMinimization(val2);
    return (val1 < val2) ? -1 : (val1 > val2) ? 1 : 0;

} // ObjectiveFunctionInfo::PreferComp



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


ObjectiveFunctionInfo::ObjectiveFunctionInfo(
    DesignTarget& target
    ) :
        InfoBase(target),
        _type(0x0)
{
    EDDY_FUNC_DEBUGSCOPE
    this->_type = new MinimizeObjectiveFunctionType(*this);

} // ObjectiveFunctionInfo::ObjectiveFunctionInfo

ObjectiveFunctionInfo::ObjectiveFunctionInfo(
    const ObjectiveFunctionInfo& copy,
    DesignTarget& target
    ) :
        InfoBase(copy, target),
        _type(0x0)
{
    EDDY_FUNC_DEBUGSCOPE
    this->_type = copy.GetType().Clone(*this);

} // ObjectiveFunctionInfo::ObjectiveFunctionInfo

ObjectiveFunctionInfo::ObjectiveFunctionInfo(
    const ObjectiveFunctionInfo& copy
    ) :
        InfoBase(copy),
        _type(0x0)
{
    EDDY_FUNC_DEBUGSCOPE
    this->_type = copy.GetType().Clone(*this);

} // ObjectiveFunctionInfo::ObjectiveFunctionInfo

ObjectiveFunctionInfo::~ObjectiveFunctionInfo(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    delete this->_type;

} // ObjectiveFunctionInfo::~ObjectiveFunctionInfo






/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA

