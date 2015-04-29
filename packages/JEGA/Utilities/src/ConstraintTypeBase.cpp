/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class ConstraintTypeBase.

    NOTES:

        See notes of ConstraintTypeBase.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jun 10 08:40:44 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the ConstraintTypeBase class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ConstraintInfo.hpp>
#include <../Utilities/include/ConstraintTypeBase.hpp>
#include <../Utilities/include/ConstraintNatureBase.hpp>
#include <../Utilities/include/NonLinearConstraintNature.hpp>



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
ConstraintTypeBase::SetNature(
    ConstraintNatureBase* nature
    )
{
    EDDY_FUNC_DEBUGSCOPE

    EDDY_ASSERT(nature != _nature);
    EDDY_ASSERT(nature != 0x0);
    EDDY_ASSERT(&nature->GetType() == this);

    if( (nature == 0x0) || (this->_nature == nature)) return false;

    delete this->_nature;
    this->_nature = nature;
    return true;

} // ConstraintTypeBase::SetNature






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
ConstraintTypeBase::GetDesignTarget(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_info.GetDesignTarget();

} // ConstraintTypeBase::GetDesignTarget

DesignTarget&
ConstraintTypeBase::GetDesignTarget(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_info.GetDesignTarget();

} // ConstraintTypeBase::GetDesignTarget

string
ConstraintTypeBase::GetNatureString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetNature().ToString();
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
ConstraintTypeBase::EvaluateConstraint(
    Design& des
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetNature().EvaluateConstraint(des);

} // ConstraintTypeBase::EvaluateConstraint

double
ConstraintTypeBase::GetPreferredAmount(
    const Design& des1,
    const Design& des2
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    double val1 = this->GetConstraintInfo().GetViolationAmount(des1);
    double val2 = this->GetConstraintInfo().GetViolationAmount(des2);

    return (val1 < val2) ? (val2 - val1) : 0.0;

} // ConstraintTypeBase::GetPreferredAmount


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

ConstraintTypeBase::ConstraintTypeBase(
    ConstraintInfo& info
    ) :
        _info(info),
        _nature(0)
{
    EDDY_FUNC_DEBUGSCOPE
    this->_nature = new NonLinearConstraintNature(*this);

} // ConstraintTypeBase::ConstraintTypeBase

ConstraintTypeBase::ConstraintTypeBase(
    const ConstraintTypeBase& copy,
    ConstraintInfo& info
    ) :
        _info(info),
        _nature(0)
{
    EDDY_FUNC_DEBUGSCOPE
    this->_nature = copy._nature->Clone(*this);

} // ConstraintTypeBase::ConstraintTypeBase

ConstraintTypeBase::~ConstraintTypeBase(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    delete this->_nature;
}





/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA
