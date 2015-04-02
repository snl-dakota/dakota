/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class MinimizeObjectiveFunctionType.

    NOTES:

        See notes of MinimizeObjectiveFunctionType.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Sep 08 12:24:58 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the MinimizeObjectiveFunctionType class.
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
#include <../Utilities/include/MinimizeObjectiveFunctionType.hpp>




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
MinimizeObjectiveFunctionType::GetName(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string name("Minimize");
    return name;

} // MinimizeObjectiveFunctionType::GetName






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
MinimizeObjectiveFunctionType::GetValueForMinimization(
    double val
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return val;

} // MinimizeObjectiveFunctionType::GetValueForMinimization

string
MinimizeObjectiveFunctionType::ToString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return GetName();

} // MinimizeObjectiveFunctionType::ToString

ObjectiveFunctionTypeBase*
MinimizeObjectiveFunctionType::Clone(
    ObjectiveFunctionInfo& forCNI
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new MinimizeObjectiveFunctionType(*this, forCNI);

} // MinimizeObjectiveFunctionType::Clone





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



MinimizeObjectiveFunctionType::MinimizeObjectiveFunctionType(
    ObjectiveFunctionInfo& info
    ) :
        ObjectiveFunctionTypeBase(info)
{
    EDDY_FUNC_DEBUGSCOPE


} // MinimizeObjectiveFunctionType::MinimizeObjectiveFunctionType

MinimizeObjectiveFunctionType::MinimizeObjectiveFunctionType(
    const MinimizeObjectiveFunctionType& copy,
    ObjectiveFunctionInfo& info
    ) :
        ObjectiveFunctionTypeBase(copy, info)
{
    EDDY_FUNC_DEBUGSCOPE


} // MinimizeObjectiveFunctionType::MinimizeObjectiveFunctionType

MinimizeObjectiveFunctionType::~MinimizeObjectiveFunctionType(
    )
{
    EDDY_FUNC_DEBUGSCOPE


} // MinimizeObjectiveFunctionType::~MinimizeObjectiveFunctionType







/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA

