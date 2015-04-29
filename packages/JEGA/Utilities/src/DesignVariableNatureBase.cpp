/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class DesignVariableNatureBase.

    NOTES:

        See notes of DesignVariableNatureBase.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jun 03 08:55:12 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the DesignVariableNatureBase class.
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
#include <../Utilities/include/RegionOfSpace.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>
#include <../Utilities/include/DesignVariableTypeBase.hpp>
#include <../Utilities/include/DesignVariableNatureBase.hpp>




/*
================================================================================
Namespace Using Directives
================================================================================
*/
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
bool
DesignVariableNatureBase::SetPrecision(
    eddy::utilities::int16_t prec
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(!this->IsPrecisionLocked());

    if(IsPrecisionLocked())
    {
        JEGALOG_II_G(lquiet(), this,
            text_entry(lquiet(), "Attempt to set precision on precision "
                "locked variable failed."
                )
            )

         return false;
    }
    this->_precision = prec;
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

double
DesignVariableNatureBase::AssertPrecision(
    double val
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    return (val==-DBL_MAX) ? -DBL_MAX : Math::Round(val, this->_precision);
}


DesignVariableInfo&
DesignVariableNatureBase::GetDesignVariableInfo(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().GetDesignVariableInfo();
}

const DesignVariableInfo&
DesignVariableNatureBase::GetDesignVariableInfo(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().GetDesignVariableInfo();
}

double
DesignVariableNatureBase::GetRandomDoubleRep(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // use the max and mins as the range.
    return this->GetRandomDoubleRep(
        this->GetMinDoubleRep(), this->GetMaxDoubleRep()
        );
}

double
DesignVariableNatureBase::GetRandomDoubleRep(
    const RegionOfSpace& within
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // we need the index of this variable which can be obtained from the
    // info object.
    size_t index = this->GetType().GetDesignVariableInfo().GetNumber();

    // use the extremes for our variable in within as the range.
    return this->GetRandomDoubleRep(
        within.GetLowerLimit(index), within.GetUpperLimit(index)
        );
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
DesignVariableNatureBase::IsValidValue(
    double value
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return (value != -DBL_MAX);
}

bool
DesignVariableNatureBase::IsValidDoubleRep(
    double rep
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return (rep != -DBL_MAX);
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
DesignVariableNatureBase::DesignVariableNatureBase(
    DesignVariableTypeBase& type
    ) :
        _type(type),
        _precision(0)
{
    EDDY_FUNC_DEBUGSCOPE
}

DesignVariableNatureBase::DesignVariableNatureBase(
    const DesignVariableNatureBase& copy,
    DesignVariableTypeBase& type
    ) :
        _type(type),
        _precision(copy._precision)
{
    EDDY_FUNC_DEBUGSCOPE
}

DesignVariableNatureBase::~DesignVariableNatureBase()
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
