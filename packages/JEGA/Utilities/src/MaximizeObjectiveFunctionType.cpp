/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class MaximizeObjectiveFunctionType.

    NOTES:

        See notes of MaximizeObjectiveFunctionType.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Sep 08 12:25:24 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the MaximizeObjectiveFunctionType class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utilities/include/Math.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ObjectiveFunctionInfo.hpp>
#include <../Utilities/include/MaximizeObjectiveFunctionType.hpp>

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

const string&
MaximizeObjectiveFunctionType::GetName(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string name("Maximize");
    return name;

} // MaximizeObjectiveFunctionType::GetName






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

double
MaximizeObjectiveFunctionType::GetValueForMinimization(
    double val
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return -val;

} // MaximizeObjectiveFunctionType::GetValueForMinimization

string
MaximizeObjectiveFunctionType::ToString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return GetName();

} // MaximizeObjectiveFunctionType::ToString

ObjectiveFunctionTypeBase*
MaximizeObjectiveFunctionType::Clone(
    ObjectiveFunctionInfo& forCNI
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new MaximizeObjectiveFunctionType(*this, forCNI);

} // MaximizeObjectiveFunctionType::Clone




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


MaximizeObjectiveFunctionType::MaximizeObjectiveFunctionType(
    ObjectiveFunctionInfo& info
    ) :
        ObjectiveFunctionTypeBase(info)
{
    EDDY_FUNC_DEBUGSCOPE


} // MaximizeObjectiveFunctionType::MaximizeObjectiveFunctionType

MaximizeObjectiveFunctionType::MaximizeObjectiveFunctionType(
    const MaximizeObjectiveFunctionType& copy,
    ObjectiveFunctionInfo& info
    ) :
        ObjectiveFunctionTypeBase(copy, info)
{
    EDDY_FUNC_DEBUGSCOPE


} // MaximizeObjectiveFunctionType::MaximizeObjectiveFunctionType

MaximizeObjectiveFunctionType::~MaximizeObjectiveFunctionType(
    )
{
    EDDY_FUNC_DEBUGSCOPE


} // MaximizeObjectiveFunctionType::~MaximizeObjectiveFunctionType






/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA

