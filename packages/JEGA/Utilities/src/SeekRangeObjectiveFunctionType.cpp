/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class SeekRangeObjectiveFunctionType.

    NOTES:

        See notes of SeekRangeObjectiveFunctionType.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Sun Oct 12 17:36:14 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the SeekRangeObjectiveFunctionType
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
#include <../Utilities/include/SeekRangeObjectiveFunctionType.hpp>






/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Utilities;
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
SeekRangeObjectiveFunctionType::GetName(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string name("Seek Range");
    return name;

} // SeekRangeObjectiveFunctionType::GetName






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
SeekRangeObjectiveFunctionType::GetValueForMinimization(
    double val
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // return the amount by which this value is out of bounds.
    return (val < _lowerBound) ? (_lowerBound - val) * (_lowerBound - val) :
           (val > _upperBound) ? (_upperBound - val) * (_upperBound - val) :
           0.0;

} // SeekRangeObjectiveFunctionType::GetValueForMinimization

string
SeekRangeObjectiveFunctionType::ToString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return GetName();

} // SeekRangeObjectiveFunctionType::ToString

ObjectiveFunctionTypeBase*
SeekRangeObjectiveFunctionType::Clone(
    ObjectiveFunctionInfo& forCNI
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new SeekRangeObjectiveFunctionType(*this, forCNI);

} // SeekRangeObjectiveFunctionType::Clone




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





SeekRangeObjectiveFunctionType::SeekRangeObjectiveFunctionType(
    ObjectiveFunctionInfo& info
    ) :
        ObjectiveFunctionTypeBase(info),
        _lowerBound(0.0),
        _upperBound(0.0)
{
    EDDY_FUNC_DEBUGSCOPE


} // SeekRangeObjectiveFunctionType::SeekRangeObjectiveFunctionType

SeekRangeObjectiveFunctionType::SeekRangeObjectiveFunctionType(
    const SeekRangeObjectiveFunctionType& copy,
    ObjectiveFunctionInfo& info
    ) :
        ObjectiveFunctionTypeBase(copy, info),
        _lowerBound(copy._lowerBound),
        _upperBound(copy._upperBound)
{
    EDDY_FUNC_DEBUGSCOPE


} // SeekRangeObjectiveFunctionType::SeekRangeObjectiveFunctionType

SeekRangeObjectiveFunctionType::~SeekRangeObjectiveFunctionType(
    )
{
    EDDY_FUNC_DEBUGSCOPE

} // SeekRangeObjectiveFunctionType::~SeekRangeObjectiveFunctionType







/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA

