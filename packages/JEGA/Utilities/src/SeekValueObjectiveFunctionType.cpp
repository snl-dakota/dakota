/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class SeekValueObjectiveFunctionType.

    NOTES:

        See notes of SeekValueObjectiveFunctionType.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Sun Oct 12 17:36:42 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the SeekValueObjectiveFunctionType
 *        class.
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
#include <../Utilities/include/SeekValueObjectiveFunctionType.hpp>


/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace eddy::utilities;







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
SeekValueObjectiveFunctionType::GetName(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string name("Seek Value");
    return name;

} // SeekValueObjectiveFunctionType::GetName







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
SeekValueObjectiveFunctionType::GetValueForMinimization(
    double val
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return (val - _value) * (val - _value);

} // SeekValueObjectiveFunctionType::GetValueForMinimization

string
SeekValueObjectiveFunctionType::ToString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return GetName();

} // SeekValueObjectiveFunctionType::ToString

ObjectiveFunctionTypeBase*
SeekValueObjectiveFunctionType::Clone(
    ObjectiveFunctionInfo& forCNI
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new SeekValueObjectiveFunctionType(*this, forCNI);

} // SeekValueObjectiveFunctionType::Clone






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





SeekValueObjectiveFunctionType::SeekValueObjectiveFunctionType(
    ObjectiveFunctionInfo& info
    ) :
        ObjectiveFunctionTypeBase(info),
        _value(0.0)
{
    EDDY_FUNC_DEBUGSCOPE


} // SeekValueObjectiveFunctionType::SeekValueObjectiveFunctionType

SeekValueObjectiveFunctionType::SeekValueObjectiveFunctionType(
    const SeekValueObjectiveFunctionType& copy,
    ObjectiveFunctionInfo& info
    ) :
        ObjectiveFunctionTypeBase(copy, info),
        _value(copy._value)
{
    EDDY_FUNC_DEBUGSCOPE


} // SeekValueObjectiveFunctionType::SeekValueObjectiveFunctionType

SeekValueObjectiveFunctionType::~SeekValueObjectiveFunctionType(
    )
{
    EDDY_FUNC_DEBUGSCOPE


} // SeekValueObjectiveFunctionType::~SeekValueObjectiveFunctionType







/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA

