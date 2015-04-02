/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class IntegerDesignVariableType.

    NOTES:

        See notes of IntegerDesignVariableType.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jun 03 08:55:28 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the IntegerDesignVariableType class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <cfloat>
#include <utilities/include/Math.hpp>
#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>
#include <../Utilities/include/DesignVariableNatureBase.hpp>
#include <../Utilities/include/IntegerDesignVariableType.hpp>



/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Logging;
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
IntegerDesignVariableType::ToString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return string("Integer");
}

bool
IntegerDesignVariableType::SetPrecision(
    eddy::utilities::int16_t prec
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(prec <= 0);
    eddy::utilities::int16_t prevPrec = GetPrecision();
    if(!DesignVariableTypeBase::SetPrecision(prec)) return false;

    if(prec > 0)
    {
        JEGALOG_II_G(lquiet(), this,
            ostream_entry(lquiet(), "Precision for integer design variable "
                "type must be <= 0.  Supplied value of ") << prec << " for "
                << GetDesignVariableInfo().GetLabel() << " rejected."
            )

        DesignVariableTypeBase::SetPrecision(prevPrec);
        return false;
    }

    return true;
}

bool
IntegerDesignVariableType::IsNatureLocked(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return false;
}

bool
IntegerDesignVariableType::IsValidDoubleRep(
    double rep
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return Math::IsWhole(rep) && DesignVariableTypeBase::IsValidDoubleRep(rep);
}

DesignVariableTypeBase*
IntegerDesignVariableType::Clone(
    DesignVariableInfo& forDVI
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new IntegerDesignVariableType(*this, forDVI);
}

double
IntegerDesignVariableType::GetValueOf(
    double rep
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return Math::IsWhole(rep) ? GetNature().GetValueOf(rep) : -DBL_MAX;
}

double
IntegerDesignVariableType::GetNearestValidValue(
    double value
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(Math::IsWhole(GetMinValue()));
    EDDY_ASSERT(Math::IsWhole(GetMaxValue()));

    if(value == -DBL_MAX) return value;

    double temp = GetNature().GetNearestValidValue(value);

    if(Math::IsWhole(temp) && IsValueInBounds(temp)) return temp;

    return GetNearestValidValue(Math::Round(temp));
}

double
IntegerDesignVariableType::GetNearestValidDoubleRep(
    double rep
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    if(rep == -DBL_MAX) return rep;

    return GetNature().GetNearestValidDoubleRep(Math::Round(rep));
}

double
IntegerDesignVariableType::GetRandomValue(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    double temp = GetNature().GetRandomValue();

    return GetNearestValidValue(Math::Round(temp));
}

double
IntegerDesignVariableType::GetDoubleRepOf(
    double value
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(Math::IsWhole(value));

    if(!Math::IsWhole(value)) return -DBL_MAX;
    return GetNature().GetDoubleRepOf(value);
}

double
IntegerDesignVariableType::GetRandomDoubleRep(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    double temp = GetNature().GetRandomDoubleRep();
    return GetNearestValidDoubleRep(temp);
}

double
IntegerDesignVariableType::GetRandomDoubleRep(
    const RegionOfSpace& within
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    double temp = GetNature().GetRandomDoubleRep(within);
    return GetNearestValidDoubleRep(temp);
}

void
IntegerDesignVariableType::SetMinValue(
    double value
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(Math::IsWhole(value));

    if(!Math::IsWhole(value))
    {
        JEGALOG_II_G(lquiet(), this,
            ostream_entry(lquiet(), "Integral lower bound value ")
                << value << " being rounded off to" << Math::Round(value)
                << "."
            )
        value = Math::Round(value);
    }
    DesignVariableTypeBase::SetMinValue(value);
}


void
IntegerDesignVariableType::SetMaxValue(
    double value
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(Math::IsWhole(value));

    if(!Math::IsWhole(value))
    {
        JEGALOG_II_G(lquiet(), this,
            ostream_entry(lquiet(), "Integral upper bound value ")
                << value << " being rounded off to" << Math::Round(value)
                << "."
            )
        value = Math::Round(value);
    }
    DesignVariableTypeBase::SetMaxValue(value);
}




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

IntegerDesignVariableType::IntegerDesignVariableType(
    DesignVariableInfo& info
    ) :
        DesignVariableTypeBase(info)
{
    EDDY_FUNC_DEBUGSCOPE
    GetNature().SetPrecision(0);
}

IntegerDesignVariableType::IntegerDesignVariableType(
    const IntegerDesignVariableType& copy,
    DesignVariableInfo& info
    ) :
        DesignVariableTypeBase(copy, info)
{
    EDDY_FUNC_DEBUGSCOPE
    GetNature().SetPrecision(0);
}






/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA
