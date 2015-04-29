/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class DesignVariableTypeBase.

    NOTES:

        See notes of DesignVariableTypeBase.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jun 03 08:55:03 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the DesignVariableTypeBase class.
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
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>
#include <../Utilities/include/DesignVariableTypeBase.hpp>
#include <../Utilities/include/DesignVariableNatureBase.hpp>
#include <../Utilities/include/ContinuumDesignVariableNature.hpp>



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
bool
DesignVariableTypeBase::SetNature(
    DesignVariableNatureBase* nature
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(!this->IsNatureLocked());
    EDDY_ASSERT(nature != this->_nature);
    EDDY_ASSERT(nature != 0x0);
    EDDY_ASSERT(& this->_nature->GetType() == this);

    if(this->IsNatureLocked() ||
        (nature == 0x0) ||
        (this->_nature == nature)) return false;

    delete this->_nature;
    this->_nature = nature;
    return true;
}






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
DesignVariableTypeBase::GetDesignTarget(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_info.GetDesignTarget();

} // DesignVariableTypeBase::GetDesignTarget

DesignTarget&
DesignVariableTypeBase::GetDesignTarget(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_info.GetDesignTarget();

} // DesignVariableTypeBase::GetDesignTarget

string
DesignVariableTypeBase::GetNatureString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return this->_nature->ToString();
}

double
DesignVariableTypeBase::AssertPrecision(
    double val
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return this->_nature->AssertPrecision(val);
}







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
DesignVariableTypeBase::IsValueInBounds(
    double value
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return this->_nature->IsValueInBounds(value);
}

bool
DesignVariableTypeBase::IsRepInBounds(
    double rep
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return this->_nature->IsRepInBounds(rep);
}

bool
DesignVariableTypeBase::IsDiscreteValueLocked(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return this->_nature->IsDiscreteValueLocked();
}

bool
DesignVariableTypeBase::IsOutOfBoundsDefined(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return this->_nature->IsOutOfBoundsDefined();
}

bool
DesignVariableTypeBase::IsPrecisionLocked(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return this->_nature->IsPrecisionLocked();
}

bool
DesignVariableTypeBase::IsValidValue(
    double value
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return this->_nature->IsValidValue(value);
}

bool
DesignVariableTypeBase::IsValidDoubleRep(
    double rep
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return (rep != -DBL_MAX) && this->_nature->IsValidDoubleRep(rep);
}

double
DesignVariableTypeBase::GetDefaultValue(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return this->_nature->GetMinValue();
}

double
DesignVariableTypeBase::GetMaxValue(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return this->_nature->GetMaxValue();
}

double
DesignVariableTypeBase::GetMinValue(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return this->_nature->GetMinValue();
}

double
DesignVariableTypeBase::GetDefaultDoubleRep(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return this->_nature->GetMinDoubleRep();
}

double
DesignVariableTypeBase::GetMaxDoubleRep(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return this->_nature->GetMaxDoubleRep();
}

double
DesignVariableTypeBase::GetMinDoubleRep(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return this->_nature->GetMinDoubleRep();
}

double
DesignVariableTypeBase::GetDistanceBetweenDoubleReps(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return this->_nature->GetDistanceBetweenDoubleReps();
}

bool
DesignVariableTypeBase::AddDiscreteValue(
    double value
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return this->_nature->AddDiscreteValue(value);
}

void
DesignVariableTypeBase::ClearDiscreteValues(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    this->_nature->ClearDiscreteValues();
}

bool
DesignVariableTypeBase::RemoveDiscreteValue(
    double value
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return this->_nature->RemoveDiscreteValue(value);
}

void
DesignVariableTypeBase::SetMinValue(
    double value
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    this->_nature->SetMinValue(value);
}

void
DesignVariableTypeBase::SetMaxValue(
    double value
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    this->_nature->SetMaxValue(value);
}

eddy::utilities::int16_t
DesignVariableTypeBase::GetPrecision(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return this->_nature->GetPrecision();
}

bool
DesignVariableTypeBase::SetPrecision(
    eddy::utilities::int16_t prec
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return this->_nature->SetPrecision(prec);
}

bool
DesignVariableTypeBase::IsDiscrete(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return this->_nature->IsDiscrete();
}

bool
DesignVariableTypeBase::IsContinuum(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_nature != 0x0);
    return this->_nature->IsContinuum();
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
DesignVariableTypeBase::DesignVariableTypeBase(
    DesignVariableInfo& info
    ) :
        _info(info),
        _nature(0x0)
{
    EDDY_FUNC_DEBUGSCOPE
    this->_nature = new ContinuumDesignVariableNature(*this);
}

DesignVariableTypeBase::DesignVariableTypeBase(
    const DesignVariableTypeBase& copy,
    DesignVariableInfo& info
    ) :
        _info(info),
        _nature(0x0)
{
    EDDY_FUNC_DEBUGSCOPE
    this->_nature = copy._nature->Clone(*this);
}

DesignVariableTypeBase::~DesignVariableTypeBase()
{
    EDDY_FUNC_DEBUGSCOPE
    delete this->_nature;
    this->_nature = 0x0;
}






/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA
