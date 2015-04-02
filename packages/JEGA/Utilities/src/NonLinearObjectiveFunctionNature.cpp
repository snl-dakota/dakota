/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class NonLinearObjectiveFunctionNature.

    NOTES:

        See notes of NonLinearObjectiveFunctionNature.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Sun Sep 14 16:39:53 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the NonLinearObjectiveFunctionNature class.
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
#include <../Utilities/include/NonLinearObjectiveFunctionNature.hpp>


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
NonLinearObjectiveFunctionNature::GetName(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string name("Non-Linear");
    return name;

} // NonLinearObjectiveFunctionNature::GetName





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
string
NonLinearObjectiveFunctionNature::ToString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return GetName();

} // NonLinearObjectiveFunctionNature::ToString

ObjectiveFunctionNatureBase*
NonLinearObjectiveFunctionNature::Clone(
    ObjectiveFunctionTypeBase& forCNI
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new NonLinearObjectiveFunctionNature(*this, forCNI);

} // NonLinearObjectiveFunctionNature::Clone

bool
NonLinearObjectiveFunctionNature::EvaluateObjective(
    Design&
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return false;

} // NonLinearObjectiveFunctionNature::EvaluateObjective





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




NonLinearObjectiveFunctionNature::NonLinearObjectiveFunctionNature(
    ObjectiveFunctionTypeBase& type
    ) :
        ObjectiveFunctionNatureBase(type)
{
    EDDY_FUNC_DEBUGSCOPE

} // NonLinearObjectiveFunctionNature::NonLinearObjectiveFunctionNature

NonLinearObjectiveFunctionNature::NonLinearObjectiveFunctionNature(
    const NonLinearObjectiveFunctionNature& copy,
    ObjectiveFunctionTypeBase& type
    ) :
        ObjectiveFunctionNatureBase(copy, type)
{
    EDDY_FUNC_DEBUGSCOPE

} // NonLinearObjectiveFunctionNature::NonLinearObjectiveFunctionNature

NonLinearObjectiveFunctionNature::~NonLinearObjectiveFunctionNature(
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

