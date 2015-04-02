/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class DesignVariableInfo.

    NOTES:

        See notes of DesignVariableInfo.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jun 03 07:34:36 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the DesignVariableInfo class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/Design.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>
#include <../Utilities/include/RealDesignVariableType.hpp>


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
void
DesignVariableInfo::SetType(
    DesignVariableTypeBase* type
    )
{
    EDDY_FUNC_DEBUGSCOPE
    delete this->_type;
    this->_type = (type == 0x0) ? new RealDesignVariableType(*this) : type;
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
bool
DesignVariableInfo::AddDiscreteValues(
    const JEGA::DoubleVector& values
    )
{
    EDDY_FUNC_DEBUGSCOPE
    bool ret = true;

    for(JEGA::DoubleVector::const_iterator it(values.begin());
        it!=values.end(); ++it) ret &= AddDiscreteValue(*it);

    return ret;
}


double
DesignVariableInfo::GetRepBoundViolation(
    double rep
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return (rep < this->GetMinDoubleRep()) ? this->GetMinDoubleRep() - rep :
           (rep > this->GetMaxDoubleRep()) ? rep - this->GetMaxDoubleRep() :
           0.0;
}

double
DesignVariableInfo::GetValueBoundViolation(
    double val
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return (val < this->GetMinValue()) ? this->GetMinValue() - val :
           (val > this->GetMaxValue()) ? val - this->GetMaxValue() :
           0.0;
}

const DesignVariableInfo&
DesignVariableInfo::operator = (
    const DesignVariableInfo &right
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(&this->GetDesignTarget() == &right.GetDesignTarget());

    if(this == &right) return *this;
    SetType(right.GetType().Clone(*this));
    return *this;
}

double
DesignVariableInfo::GetNearestValidValue(
    double value
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().GetNearestValidValue(value);
}

double
DesignVariableInfo::GetNearestValidDoubleRep(
    double rep
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetType().GetNearestValidDoubleRep(rep);
}

double
DesignVariableInfo::WhichValue(
    const Design& des
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return des.GetVariableValue(this->GetNumber());
}

double
DesignVariableInfo::WhichDoubleRep(
    const Design& des
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return des.GetVariableRep(this->GetNumber());
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
DesignVariableInfo::DesignVariableInfo(
    DesignTarget& target
    ) :
        InfoBase(target),
        _type(0x0)
{
    EDDY_FUNC_DEBUGSCOPE
    this->_type = new RealDesignVariableType(*this);
}

DesignVariableInfo::DesignVariableInfo(
    const DesignVariableInfo& copy,
    DesignTarget& target
    ) :
        InfoBase(copy, target),
        _type(0x0)
{
    EDDY_FUNC_DEBUGSCOPE
    this->_type = copy.GetType().Clone(*this);

} // DesignVariableInfo::DesignVariableInfo

DesignVariableInfo::DesignVariableInfo(
    const DesignVariableInfo& copy
    ) :
        InfoBase(copy),
        _type(0x0)
{
    EDDY_FUNC_DEBUGSCOPE
    this->_type = copy.GetType().Clone(*this);
}

DesignVariableInfo::~DesignVariableInfo(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    delete this->_type;
}







/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA
