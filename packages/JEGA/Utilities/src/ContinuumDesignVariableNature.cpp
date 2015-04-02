/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class ContinuumDesignVariableNature.

    NOTES:

        See notes of ContinuumDesignVariableNature.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jun 03 08:55:45 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the ContinuumDesignVariableNature
 *        class.
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
#include <utilities/include/RandomNumberGenerator.hpp>
#include <../Utilities/include/ContinuumDesignVariableNature.hpp>


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
string
ContinuumDesignVariableNature::ToString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return string("Continuum");
}

DesignVariableNatureBase*
ContinuumDesignVariableNature::Clone(
    DesignVariableTypeBase& forType
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new ContinuumDesignVariableNature(*this, forType);
}

double
ContinuumDesignVariableNature::GetMaxDoubleRep(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_maxVal;
}

double
ContinuumDesignVariableNature::GetMinDoubleRep(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_minVal;
}

double
ContinuumDesignVariableNature::GetRandomDoubleRep(
    double lb,
    double ub
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return RandomNumberGenerator::UniformReal(lb, ub);
}

double
ContinuumDesignVariableNature::GetDoubleRepOf(
    double value
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return value;
}

double
ContinuumDesignVariableNature::GetRandomValue(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return RandomNumberGenerator::UniformReal(
        this->GetMinValue(), this->GetMaxValue()
        );
}

double
ContinuumDesignVariableNature::GetMaxValue(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_maxVal;
}

double
ContinuumDesignVariableNature::GetMinValue(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_minVal;
}

double
ContinuumDesignVariableNature::GetValueOf(
    double rep
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return rep;
}

double
ContinuumDesignVariableNature::GetDistanceBetweenDoubleReps(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return Math::Pow(10, static_cast<double>(-this->GetPrecision()));
}

bool
ContinuumDesignVariableNature::AddDiscreteValue(
    double
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II_G_F(this,
        text_entry(lfatal(), this->GetDesignVariableInfo().GetLabel() +
            ": Continuum natured variable cannot accept discrete values.")
        )

    return false;
}

void
ContinuumDesignVariableNature::ClearDiscreteValues(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II_G_F(this,
        text_entry(lfatal(), this->GetDesignVariableInfo().GetLabel() +
            ": Cannot clear discrete values for continuum natured variable.")
        )
}

bool
ContinuumDesignVariableNature::RemoveDiscreteValue(
    double
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II_G_F(this,
        text_entry(lfatal(), this->GetDesignVariableInfo().GetLabel() +
            ": Continuum natured variable has no discrete values to remove.")
        )

    return false;
}

void
ContinuumDesignVariableNature::SetMaxValue(
    double value
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(value != -DBL_MAX);
    this->_maxVal = value;
}

void
ContinuumDesignVariableNature::SetMinValue(
    double value
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(value != -DBL_MAX);
    this->_minVal = value;
}

bool
ContinuumDesignVariableNature::IsDiscreteValueLocked(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return true;
}

bool
ContinuumDesignVariableNature::IsValueInBounds(
    double value
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->GetMinValue() <= this->GetMaxValue());
    return (value >= this->GetMinValue()) && (value <= this->GetMaxValue());
}

bool
ContinuumDesignVariableNature::IsRepInBounds(
    double rep
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->GetMinDoubleRep() <= this->GetMaxDoubleRep());
    return (rep >= this->GetMinDoubleRep()) && (rep <= this->GetMaxDoubleRep());
}

bool
ContinuumDesignVariableNature::IsOutOfBoundsDefined(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return true;
}

bool
ContinuumDesignVariableNature::IsPrecisionLocked(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return false;
}

bool
ContinuumDesignVariableNature::IsValidValue(
    double value
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->IsValueInBounds(value);
}

bool
ContinuumDesignVariableNature::IsValidDoubleRep(
    double rep
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->IsRepInBounds(rep);
}

bool
ContinuumDesignVariableNature::IsDiscrete(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return false;
}

bool
ContinuumDesignVariableNature::IsContinuum(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return true;
}

double
ContinuumDesignVariableNature::GetNearestValidValue(
    double value
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // The best guess that this nature can make is
    // to return something in bounds
    return Math::Max(Math::Min(value, this->GetMaxValue()), this->GetMinValue());
}

double
ContinuumDesignVariableNature::GetNearestValidDoubleRep(
    double rep
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // The best guess that this nature can make is
    // to return something in bounds
    return Math::Max(
        Math::Min(rep, this->GetMaxDoubleRep()),
        this->GetMinDoubleRep()
        );
}





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
ContinuumDesignVariableNature::ContinuumDesignVariableNature(
    DesignVariableTypeBase& type
    ) :
        DesignVariableNatureBase(type),
        _maxVal(0.0),
        _minVal(0.0)
{
    EDDY_FUNC_DEBUGSCOPE
}

ContinuumDesignVariableNature::ContinuumDesignVariableNature(
    const ContinuumDesignVariableNature& copy,
    DesignVariableTypeBase& type
    ) :
        DesignVariableNatureBase(copy, type),
        _maxVal(0.0),
        _minVal(0.0)
{
    EDDY_FUNC_DEBUGSCOPE
}

ContinuumDesignVariableNature::~ContinuumDesignVariableNature(
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
