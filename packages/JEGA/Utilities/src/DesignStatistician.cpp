/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class DesignStatistician.

    NOTES:

        See notes of DesignStatistician.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 22 07:45:06 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the DesignStatistician class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <algorithm>
#include <../Utilities/include/Design.hpp>
#include <../Utilities/include/Design.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <../Utilities/include/DesignTarget.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ConstraintInfo.hpp>
#include <../Utilities/include/DesignGroupVector.hpp>
#include <../Utilities/include/DesignStatistician.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>


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
double
DesignStatistician::ComputeTotalPercentageViolation(
    const Design& des
    )
{
    EDDY_FUNC_DEBUGSCOPE

    return ComputeNonSidePercentageViolation(des) +
           ComputeSidePercentageViolation(des);

} // DesignStatistician::ComputeTotalPercentageViolation


double
DesignStatistician::ComputeNonSidePercentageViolation(
    const Design& des
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // prepare to iterate the constraint infos and sum the percentage
    // violations.
    const ConstraintInfoVector& infos =
        des.GetDesignTarget().GetConstraintInfos();

    if(infos.size() == 0) return 0.0;

    // now iterate and sum.
    double violp = 0.0;

    ConstraintInfoVector::const_iterator it(infos.begin());
    for(; it!=infos.end(); ++it) violp += (*it)->GetViolationPercentage(des);

    // now just return the result.
    return violp;

} // DesignStatistician::ComputeNonSidePercentageViolation


double
DesignStatistician::ComputeSidePercentageViolation(
    const Design& des
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // prepare to iterate the Design variable infos and compute the
    // percentage violations.
    const DesignVariableInfoVector& infos =
        des.GetDesignTarget().GetDesignVariableInfos();

    // prepare to store the violation percentage summation.
    double violp = 0.0;

    // iterate them and compute the sum-of percentage violations.
    for(size_t i=0; i<infos.size(); ++i)
    {
        const DesignVariableInfo& info = *infos[i];

        // start by getting the amount of bound violation.
        double cviol = info.GetRepBoundViolation(des);

        // if there is none, we can move on.
        if(cviol == 0.0) continue;

        // now get the range which is what we will consider a 100% violation.
        double range = info.GetDoubleRepRange();

        // if the range is for some reason 0, we add a violation equal to the
        // rep value since we cannot really compute it.
        if(range == 0.0) cviol = Math::Abs(
            info.WhichDoubleRep(des) - info.GetMinDoubleRep()
            );

        // otherwise, we add in our current violation percentage.
        else cviol = cviol/range * 100.0;

        violp += cviol; //= (violp == 0.0) ? cviol : violp * cviol;
    }

    // now return our result.
    return violp;

} // DesignStatistician::ComputeSidePercentageViolation

DesignOFSortSet
DesignStatistician::CollectDesignsByOF(
    const DesignGroupVector& groups
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // If there are no groups, return an empty set.
    if(groups.empty()) return DesignOFSortSet();

    // Use the first group to initialize the set.  After that, insert each in.
    groups[0]->SynchronizeOFAndDVContainers();
    DesignOFSortSet ret(groups[0]->GetOFSortContainer());

    // Iterate the remaining groups, synchronize their sorts, and then
    // put all their OF designs into the returned container.
    for(size_t i=1; i<groups.size(); ++i)
    {
        groups[i]->SynchronizeOFAndDVContainers();
        ret.insert(groups[i]->BeginOF(), groups[i]->EndOF());
    }
    return ret;
}

DesignDVSortSet
DesignStatistician::CollectDesignsByDV(
    const DesignGroupVector& groups
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Iterate the groups, synchronize their sorts, and then
    // put all their DV designs into the returned container.
    DesignDVSortSet ret;

    for(DesignGroupVector::const_iterator it(groups.begin());
        it!=groups.end(); ++it)
    {
        (*it)->SynchronizeOFAndDVContainers();
        ret.insert((*it)->BeginDV(), (*it)->EndDV());
    }
    return ret;
}

int
DesignStatistician::ParetoConstraintCompare(
   const Design& des1,
   const Design& des2
   )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(des1.IsEvaluated());
    EDDY_ASSERT(des2.IsEvaluated());

    // If they are the same Design, we needn't go any further.
    if(&des1 == &des2) return 0;

    // If one Design is feasible and the other is not
    // we needn't go any further.
    if(des1.IsFeasible() != des2.IsFeasible())
        return des1.IsFeasible() ? -1 : 1;

    // by now, we know that either both are feasible or both are
    // infeasible.  If both are feasible, we can get out.
    if(des1.IsFeasible()) return 0;

    // If we make it here, both are infeasible.

    // Give priority to bound constraints
    if(des1.SatisfiesBounds() != des2.SatisfiesBounds())
        return des1.SatisfiesBounds() ? -1 : 1;

    // By now, we know that either both designs satisfy the bound
    // constraints or neither of them do.  We can find out which by
    // looking at either of them.
    bool bfeas = des1.SatisfiesBounds();

    // Get the target from one of the Designs.
    // we need it to get the information objects.
    const DesignTarget& target = des1.GetDesignTarget();

    // assert that they are both using the same target.
    EDDY_ASSERT(&des2.GetDesignTarget() == &target);

    // Do constraint-by-constraint comparison.
    // Bound first if applicable and then non-bound.

    // Prepare to store the winner.
    const Design* whois = 0x0;
    const Design* whowas = 0x0;

    // only need to do this if neither Design satisfies the
    // bound constraints.
    if(!bfeas)
    {
        // Get the DesignVariableInfo's from the target
        const DesignVariableInfoVector& dvinfos =
            target.GetDesignVariableInfos();

        // prepare to iterate the infos.
        DesignVariableInfoVector::const_iterator dit(dvinfos.begin());

        for(; dit!=dvinfos.end(); ++dit)
        {
            // Get the violation amounts to compare.
            const double d1viol = (*dit)->GetRepBoundViolation(des1);
            const double d2viol = (*dit)->GetRepBoundViolation(des2);

            // This says that if the violations are equal, whois is null.
            // otherwise it points to the design with the lesser violation
            // amount.
            whois = (d1viol == d2viol) ? 0x0 :
                    (d1viol <  d2viol) ? &des1 : &des2;

            // The logic below says the following:
            //   If des1 is better for this constraint
            //      And des2 was better for a previous constraint
            //   Then neither is a possible winner and we return equal.
            //
            //   Else If des2 is better for this constraint
            //      And des1 was better for a previous constraint
            //   Then neither is a possible winner and we return equal.
            //
            //   Otherwise, they are equal for this constraint and whoever
            //   was previously better is still better.
            if(whois == &des1) {
                if(whowas == &des2) return 0;
            }
            else if(whois == &des2) {
                if(whowas == &des1) return 0;
            }
            else whois = whowas;
            whowas = whois;
        }
    }

    // if we make it here, we have not identified a non-resolvable
    // tie.  A non-resolvable tie occurs when neither Design is better
    // and they do not have all equal violations.

    // So we will move on to comparing the non-bound constraint values.

    // Get the constraint information objects from the target.
    const ConstraintInfoVector& cinfos = target.GetConstraintInfos();

    // NOTE: if there are no constraints, then we know that both Designs
    // violate the bound constraints.  Otherwise they would be feasible.
    EDDY_ASSERT(!(cinfos.empty() && bfeas));

    // we obviously only need to do this if there are constraints.
    if(!cinfos.empty())
    {
        // Get iterators for traversing constraint infos.
        ConstraintInfoVector::const_iterator cit(cinfos.begin());

        for(; cit!=cinfos.end(); ++cit)
        {
            whois = (*cit)->GetPreferredDesign(des1, des2);

            if(whois == &des1) {
                if(whowas == &des2) return 0;
            }
            else if(whois == &des2) {
                if(whowas == &des1) return 0;
            }
            else whois = whowas;
            whowas = whois;
        }
    }

    // now we return the winner if we identified one or
    // we return 0 to indicate no winner.
    return (whois == 0x0) ? 0 : (whois == &des1) ? -1 : 1;

} // DesignStatistician::ParetoConstraintCompare

eddy::utilities::DoubleExtremes
DesignStatistician::GetDesignVariableExtremes(
    const DesignDVSortSet& from
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // If from is emtpy, we cannot return anything sensible.
    if(from.empty()) return eddy::utilities::DoubleExtremes();

    // retrieve the number of objective functions.
    const std::size_t ndv = from.front()->GetNDV();

    // prepare a return object.
    eddy::utilities::DoubleExtremes ret(ndv, DBL_MAX, -DBL_MAX);

    // Iterate through and find the maxs/mins for each of the objectives.
    DesignDVSortSet::const_iterator it(from.begin());
    const DesignDVSortSet::const_iterator e(from.end());

    ret.set_min(0, (*it)->GetVariableRep(0));
    ret.set_max(0, (*from.rend())->GetVariableRep(0));

    // really what we are doing is storing the highest
    // and lowest values in this set for each variable.
    for(; it!=e; ++it)
        for(std::size_t dv=1; dv<ndv; ++dv)
            ret.take_if_either(dv, (*it)->GetVariableRep(dv));

    return ret;
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
