/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class UnconstrainedStatistician.

    NOTES:

        See notes of UnconstrainedStatistician.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 22 08:05:53 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the UnconstrainedStatistician class.
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
#include <../Utilities/include/DesignMultiSet.hpp>
#include <../Utilities/include/DesignValueMap.hpp>
#include <../Utilities/include/ConstraintInfo.hpp>
#include <../Utilities/include/UnconstrainedStatistician.hpp>


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
Utility Class Definitions
================================================================================
*/
bool
UnconstrainedStatistician::Penalizer::CheckDesign(
    const Design* des
    )
{
    EDDY_FUNC_DEBUGSCOPE

    bool evald = des->IsEvaluated();
    bool illCond = des->IsIllconditioned();

    JEGAIFLOG_CF_IT_G(!evald, lquiet(), UnconstrainedStatistician,
        text_entry(lquiet(), "Applying penalty to non-evaluated Design.  "
            "Result may be nonsensical.")
        )

    JEGAIFLOG_CF_IT_G(illCond, lquiet(), UnconstrainedStatistician,
        text_entry(lquiet(), "Applying penalty to illconditioned Design.  "
            "Result may be nonsensical.")
        )

    return evald && !illCond;
}

double
UnconstrainedStatistician::BoundsPenalizer::operator()(
    const Design* des
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(CheckDesign(des));

    const DesignVariableInfoVector& infos =
        des->GetDesignTarget().GetDesignVariableInfos();
    size_t ndv = infos.size();

    double pen2Add = 0.0;

    for(size_t i=0; i<ndv; ++i)
    {
        double above = infos[i]->GetMaxDoubleRep() - des->GetVariableRep(i);
        if(above < 0.0) { pen2Add += above * above; continue; }

        double below = des->GetVariableRep(i) - infos[i]->GetMinDoubleRep();
        if(below < 0.0) pen2Add += below * below;
    }

    return pen2Add;
}

double
UnconstrainedStatistician::ExteriorPenalizer::operator()(
    const Design* des
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(CheckDesign(des));

    const ConstraintInfoVector& infos =
        des->GetDesignTarget().GetConstraintInfos();
    const size_t ncn = infos.size();

    double penalty = 0.0;
    for(size_t i=0; i<ncn; ++i)
    {
        const double viol = infos[i]->GetViolationAmount(*des);
        penalty += viol * viol;
    }

    return this->_multiplier * penalty;
}






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

DesignDoubleMap
UnconstrainedStatistician::ApplyExteriorPenalty(
    const DesignGroup& designs,
    double multiplier
    )
{
    EDDY_FUNC_DEBUGSCOPE

    return ApplyPenalty<ExteriorPenalizer>(
        designs, ExteriorPenalizer(multiplier)
        );
}

DesignDoubleMap
UnconstrainedStatistician::ApplyExteriorPenalty(
    const DesignGroupVector& designs,
    double multiplier
    )
{
    EDDY_FUNC_DEBUGSCOPE

    return ApplyPenalty<ExteriorPenalizer>(
        designs, ExteriorPenalizer(multiplier)
        );
}

DesignDoubleMap
UnconstrainedStatistician::ApplyQuadraticPenalty(
    const DesignGroup& designs,
    double multiplier
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ApplyExteriorPenalty(designs, multiplier);
}

DesignDoubleMap
UnconstrainedStatistician::ApplyQuadraticPenalty(
    const DesignGroupVector& designs,
    double multiplier
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ApplyExteriorPenalty(designs, multiplier);
}

const DesignDoubleMap&
UnconstrainedStatistician::AddOutOfBoundsPenalties(
    const DesignGroup& designs,
    DesignDoubleMap& to
    )
{
    EDDY_FUNC_DEBUGSCOPE
    if(designs.IsEmpty()) return to;
    return AddPenalty<BoundsPenalizer>(designs, BoundsPenalizer(), to);
}

const DesignDoubleMap&
UnconstrainedStatistician::AddOutOfBoundsPenalties(
    const DesignGroupVector& designs,
    DesignDoubleMap& to
    )
{
    EDDY_FUNC_DEBUGSCOPE
    if(designs.empty()) return to;
    return AddPenalty<BoundsPenalizer>(designs, BoundsPenalizer(), to);
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





/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA
