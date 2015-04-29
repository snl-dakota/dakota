/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class BooleanDesignVariableType.

    NOTES:

        See notes of BooleanDesignVariableType.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.1.0

    CHANGES:

        Thu Oct 26 15:34:46 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the BooleanDesignVariableType class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/RegionOfSpace.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>
#include <utilities/include/RandomNumberGenerator.hpp>
#include <../Utilities/include/DesignVariableNatureBase.hpp>
#include <../Utilities/include/BooleanDesignVariableType.hpp>







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
BooleanDesignVariableType::ToString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return string("Boolean");
}

bool
BooleanDesignVariableType::AddDiscreteValue(
    double value
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II_G(lquiet(), this,
        ostream_entry(lquiet(), "Attempt to add a discrete value of ")
            << value << " to a Boolean design variable failed.  0 and 1 are "
            "the only allowed values.  New ones cannot be added."
        )

    return false;
}

bool
BooleanDesignVariableType::SetPrecision(
    eddy::utilities::int16_t prec
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(prec == 0);
    if(!this->DesignVariableTypeBase::SetPrecision(prec)) return false;

    if(prec != 0)
    {
        JEGALOG_II_G(lquiet(), this,
            ostream_entry(lquiet(), "Precision for Boolean design variable "
                "type must be 0.  Supplied value of ") << prec << " for "
                << this->GetDesignVariableInfo().GetLabel() << " rejected."
            )

        this->DesignVariableTypeBase::SetPrecision(0);
        return false;
    }

    return true;
}

bool
BooleanDesignVariableType::IsNatureLocked(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return false;
}

bool
BooleanDesignVariableType::IsValidDoubleRep(
    double rep
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return true;
}

DesignVariableTypeBase*
BooleanDesignVariableType::Clone(
    DesignVariableInfo& forDVI
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new BooleanDesignVariableType(*this, forDVI);
}

double
BooleanDesignVariableType::GetValueOf(
    double rep
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return rep == 0.0 ? 0.0 : 1.0;
}

double
BooleanDesignVariableType::GetNearestValidValue(
    double value
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(Math::IsWhole(GetMinValue()));
    EDDY_ASSERT(Math::IsWhole(GetMaxValue()));
    return value == 0.0 ? 0.0 : 1.0;
}

double
BooleanDesignVariableType::GetNearestValidDoubleRep(
    double rep
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return rep == 0.0 ? 0.0 : 1.0;
}

double
BooleanDesignVariableType::GetRandomValue(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return RandomNumberGenerator::RandomBoolean();
}

double
BooleanDesignVariableType::GetDoubleRepOf(
    double value
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return value == 0.0 ? 0.0 : 1.0;
}

double
BooleanDesignVariableType::GetRandomDoubleRep(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return RandomNumberGenerator::RandomBoolean();
}

double
BooleanDesignVariableType::GetRandomDoubleRep(
    const RegionOfSpace& within
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    const size_t index = this->GetDesignVariableInfo().GetNumber();

    bool lb = this->GetNearestValidDoubleRep(
        within.GetLowerLimit(index)
        ) == 0.0;
    bool ub = this->GetNearestValidDoubleRep(
        within.GetUpperLimit(index)
        ) == 0.0;

    return RandomNumberGenerator::RandomBoolean() ? lb : ub;
}

void
BooleanDesignVariableType::SetMinValue(
    double value
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(value == 0.0);

    if(value != 0.0)
    {
        JEGALOG_II_G(lquiet(), this,
            ostream_entry(lquiet(), "Boolean lower bound value must be 0.  "
                "rejecting supplied value of ") << value << "."
            )
    }
    this->DesignVariableTypeBase::SetMinValue(0.0);
}


void
BooleanDesignVariableType::SetMaxValue(
    double value
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(value == 1.0);

    if(value != 1.0)
    {
        JEGALOG_II_G(lquiet(), this,
            ostream_entry(lquiet(), "Boolean lower bound value must be 1.  "
                "rejecting supplied value of ") << value << "."
            )
    }
    this->DesignVariableTypeBase::SetMinValue(1.0);
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

BooleanDesignVariableType::BooleanDesignVariableType(
    DesignVariableInfo& info
    ) :
        DesignVariableTypeBase(info)
{
    EDDY_FUNC_DEBUGSCOPE
    this->GetNature().SetPrecision(0);
}

BooleanDesignVariableType::BooleanDesignVariableType(
    const BooleanDesignVariableType& copy,
    DesignVariableInfo& info
    ) :
        DesignVariableTypeBase(copy, info)
{
    EDDY_FUNC_DEBUGSCOPE
    this->GetNature().SetPrecision(0);
}







/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA

