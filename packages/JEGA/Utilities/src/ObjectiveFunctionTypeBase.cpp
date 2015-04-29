/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class ObjectiveFunctionTypeBase.

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
 * \brief Contains the implementation of the ObjectiveFunctionTypeBase class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ObjectiveFunctionInfo.hpp>
#include <../Utilities/include/ObjectiveFunctionTypeBase.hpp>
#include <../Utilities/include/ObjectiveFunctionNatureBase.hpp>
#include <../Utilities/include/NonLinearObjectiveFunctionNature.hpp>


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
ObjectiveFunctionTypeBase::GetDesignTarget(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_info.GetDesignTarget();

} // ObjectiveFunctionTypeBase::GetDesignTarget

DesignTarget&
ObjectiveFunctionTypeBase::GetDesignTarget(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_info.GetDesignTarget();

} // ObjectiveFunctionTypeBase::GetDesignTarget

string
ObjectiveFunctionTypeBase::GetNatureString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetNature().ToString();

} // ObjectiveFunctionTypeBase::GetNatureString

bool
ObjectiveFunctionTypeBase::SetNature(
    ObjectiveFunctionNatureBase* nature
    )
{
    EDDY_FUNC_DEBUGSCOPE

    EDDY_ASSERT(nature != _nature);
    EDDY_ASSERT(nature != 0x0);
    EDDY_ASSERT(&nature->GetType() == this);

    if( (nature == 0x0) || (this->_nature == nature)) return false;

    delete _nature;
    this->_nature = nature;
    return true;

} // ObjectiveFunctionTypeBase::SetNature






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

bool
ObjectiveFunctionTypeBase::EvaluateObjective(
    Design& des
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetNature().EvaluateObjective(des);

} // ObjectiveFunctionTypeBase::EvaluateObjective

double
ObjectiveFunctionTypeBase::GetPreferredAmount(
    double val1,
    double val2
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    return this->GetValueForMinimization(val2) -
        this->GetValueForMinimization(val1);

} // ObjectiveFunctionTypeBase::GetPreferredAmount


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



ObjectiveFunctionTypeBase::ObjectiveFunctionTypeBase(
    ObjectiveFunctionInfo& info
    ) :
        _info(info),
        _nature(0)
{
    EDDY_FUNC_DEBUGSCOPE
    _nature = new NonLinearObjectiveFunctionNature(*this);

} // ObjectiveFunctionTypeBase::ObjectiveFunctionTypeBase

ObjectiveFunctionTypeBase::ObjectiveFunctionTypeBase(
    const ObjectiveFunctionTypeBase& copy,
    ObjectiveFunctionInfo& info
    ) :
        _info(info),
        _nature(0)
{
    EDDY_FUNC_DEBUGSCOPE
    _nature = copy._nature->Clone(*this);

} // ObjectiveFunctionTypeBase::ObjectiveFunctionTypeBase

ObjectiveFunctionTypeBase::~ObjectiveFunctionTypeBase(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    delete _nature;
}






/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA

