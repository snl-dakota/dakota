/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class MultiObjectiveStatistician.

    NOTES:

        See notes of MultiObjectiveStatistician.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 22 08:02:26 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the MultiObjectiveStatistician class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignValueMap.hpp>
#include <../Utilities/include/DesignStatistician.hpp>
#include <../Utilities/include/DesignDoubleValueMap.hpp>
#include <../Utilities/include/ObjectiveFunctionInfo.hpp>
#include <../Utilities/include/MultiObjectiveStatistician.hpp>


/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA;
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
const size_t MultiObjectiveStatistician::MARK = 6;







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

int
MultiObjectiveStatistician::DominationCompare(
    const Design& des1,
    const Design& des2,
    const ObjectiveFunctionInfoVector& infos
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // If des1 and des2 are the same design, skip this check.
    if(&des1 == &des2) return 0;

    // a feasible design always dominates an infeasible design.
    if(des1.IsFeasible() != des2.IsFeasible())
        return des1.IsFeasible() ? -1 : 1;

    // get the design target for use below.
    EDDY_DEBUGEXEC(const DesignTarget& target = des1.GetDesignTarget())

    // make sure that both designs are for the same target.
    EDDY_ASSERT(&des2.GetDesignTarget() == &target);

    // if the designs are not feasible, then preference will be given
    // to finding feasibility so the constraints will be compared first.
    // Note that des1's feasiblilty is representative of both
    // Designs feasibility (see beginning of method).
    if(!des1.IsFeasible())
    {
        // Compare the total violations of the two Designs.
        const double d1cv =
			DesignStatistician::ComputeTotalPercentageViolation(des1);
        const double d2cv =
			DesignStatistician::ComputeTotalPercentageViolation(des2);

        // see if we can identify a winner based on these violations.
        if(d1cv < d2cv) return -1;
        if(d1cv > d2cv) return 1;
    }

    // Note, by now, we have two feasible Designs or neither is preferred by
    // constraint.

    // Now prepare to iterate the objective infos.  To possibly reduce the
    // amount of comparison, we will look for a "flip" in preference from one
    // Design to another.  If we find such a flip, we will exit with equal
    // status.

    // So we will need a variable to store the current preference
    int current = 0;

    // iterate the objective functions and try to determine who's the best.
    for(std::size_t i=0; i<infos.size(); ++i)
    {
        // figure out who is preferred for this objective.
        const Design* prefDes = infos[i]->GetPreferredDesign(des1, des2);

        // store the preferred Design in our language of -1, 0, 1
        int newPref = (prefDes == &des1) ? -1 : (prefDes == &des2) ? 1 : 0;

        // if current is zero, accept newPref no matter what and continue
        if(current == 0) current = newPref;

        // else if current is the opposite of newPref, we have equal designs
        // and we can get out with that information.
        else if(newPref == -current) return 0;
    }

    // if we make it here, we have had no flip and we either have a preferred
    // Design or we have equality.  Either way we return our current.
    return current;

} // MultiObjectiveStatistician::DominationCompare

std::size_t
MultiObjectiveStatistician::FlushDominatedFrom(
    DesignGroup& group
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // begin by "unmarking" all of group.
    for(DesignOFSortSet::const_iterator it(group.BeginOF());
        it!=group.EndOF(); ++it)
            (*it)->ModifyAttribute(MARK, false);

    // now separate the dominated from it.
    DesignOFSortSet dom(GetDominated(group.GetOFSortContainer()));

    // go through the dominated and mark them
    size_t nMarked = 0;
    for(DesignOFSortSet::const_iterator it(dom.begin()); it!=dom.end(); ++it)
    {
        (*it)->ModifyAttribute(MARK, true);
        ++nMarked;
    }

    // now flush the dominated from the group and return the number removed.
    return nMarked > 0 ? group.FlushDesigns(MARK) : 0;

} // MultiObjectiveStatistician::FlushDominatedFrom

eddy::utilities::DoubleExtremes
MultiObjectiveStatistician::FindParetoExtremes(
    const DesignOFSortSet& of
    )
{
    EDDY_FUNC_DEBUGSCOPE

    //// this only counts feasible Designs.  So if there
    //// aren't any, get out.
    //if(DesignStatistician::FindFeasible(of) == of.end())
    //    return DoubleExtremes();

    // now get the non-dominated from "of".  Since we verified that
    // there are feasible designs above, all of nondom will be feasible.
    // we will pass it directly into the regular DesignStatistician method
    // for finding objective extremes.
    return DesignStatistician::GetObjectiveFunctionExtremes(
        GetNonDominated(of)
        );

} // MultiObjectiveStatistician::FindParetoExtremes

DesignOFSortSet::const_iterator
MultiObjectiveStatistician::FindDominatedDesign(
    const Design& des,
    const DesignOFSortSet& in,
    DesignOFSortSet::const_iterator where
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // take the users word for it and only look after where.
    // we will begin by assuming that all designs are feasible and
    // testing the minimal region.
    DesignOFSortSet::const_iterator found(
        FindDominatedDesign(des, in, where, false)
        );

    // if it didn't find one, we will look before where for an infeasible
    // design.  If we find it, it is the bad guy.  If not, we have no bad guy.
    return found == in.end() ?
        DesignStatistician::FindNonFeasible(in.begin(), where) : found;

} // MultiObjectiveStatistician::FindDominatedDesign

DesignOFSortSet::const_iterator
MultiObjectiveStatistician::FindDominatedDesign(
    const Design& des,
    const DesignOFSortSet& in,
    DesignOFSortSet::const_iterator where,
    bool hasInfeasible
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(des.IsEvaluated());

    // check for trivial abort conditions.
    if(in.empty() || !des.IsEvaluated()) return in.end();

    // prepare to iterate.
    DesignOFSortSet::const_iterator it(in.begin());
    DesignOFSortSet::const_iterator e(in.end());

    // if in is all feasible, we can take a shortcut.
    if(!hasInfeasible)
    {
        // all of "in" are feasible and "des" is not
        // then "des" doesn't dominate any of them.
        if(!des.IsFeasible()) return e;

        // otherwise we need only look beyond the proper location for "des".
        it = (where == in.begin()) ?
            in.upper_bound( &const_cast<Design&>(des) ) :
            where;

        // if "it" is the end, we can get out here because
        // a Design can only dominate another further down
        // (toward end) the list if they are all feasible.
        if(it == e) return e;
    }

    // otherwise if "in" has infeasible and "des" is feasible
    // we can bound our region with feas and the proper location of "des".
    else if(des.IsFeasible())
    {
        // find "des"'s proper location in "in".
        it = (where == in.begin()) ?
            in.upper_bound( &const_cast<Design&>(des) ) :
            where;

        // we'll need the location of that infeasible design
        DesignOFSortSet::const_iterator infeas =
            DesignStatistician::FindNonFeasible(in);

        // if it is the end of in, we can return from here
        // with the location of the first infeasible.
        if(it == e) return infeas;

        // if the location of the infeasible design is after "it", we needn't
        // look beyond infeas (but we must look at it) because it will be
        // dominated by des.
        if(in.key_comp()(*it, *infeas)) e = ++infeas;
    }

    // des can only dominate those in the range [it, e).

    // store the objective function infos for repeated
    // use in the domination compare function
    const ObjectiveFunctionInfoVector& infos =
        des.GetDesignTarget().GetObjectiveFunctionInfos();

    // now loop through [it, e).
    for(; it!=e; ++it)
        if(DominationCompare(des, *(*it), infos) == -1) return it;

    // if we get here, we were unable to dominate any designs at all.
    return in.end();

} // MultiObjectiveStatistician::FindDominatedDesign

DesignOFSortSet::const_iterator
MultiObjectiveStatistician::FindDominatingDesign(
    const Design& des,
    const DesignOFSortSet& in,
    DesignOFSortSet::const_iterator where
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(des.IsEvaluated());

    // Check for trivial abort conditions
    if(in.empty() || !des.IsEvaluated()) return in.end();

    // e will store the last location we need look at
    DesignOFSortSet::const_iterator e(in.end());

    // a feasible design can only be dominated by one
    // further up the list (closer to begin()).
    if(des.IsFeasible())
    {
        e = (where == in.end()) ?
            in.lower_bound( const_cast<Design*>(&des) ) :
            where;

        // if e is the beginning, we can end now because des
        // cannot be dominated.
        if(e == in.begin()) return in.end();
    }
    // an infeasible Design is dominated by any feasible Design
    else
    {
        DesignOFSortSet::const_iterator feas(
            DesignStatistician::FindFeasible(in)
            );
        if(feas != in.end()) return feas;

        // otherwise all designs are infeasible and this
        // is going to be an expensive search.
    }

    // now we have the search region bounded by begin() and e. Check each design
    // in that range and return if one of them dominates des.
    DesignOFSortSet::const_iterator it(in.begin());

    // store the objective function infos for repeated
    // use in the domination compare function
    const ObjectiveFunctionInfoVector& infos =
        des.GetDesignTarget().GetObjectiveFunctionInfos();

    // iterate the range [it, e).
    for(; it!=e; ++it)
        if(DominationCompare(des, **it, infos) == 1) return it;

    // if we get here, there are no dominating designs for des.
    return in.end();

} // MultiObjectiveStatistician::FindDominatingDesign

DesignOFSortSet::const_iterator
MultiObjectiveStatistician::FindMostDominated(
    const Design& by,
    const DesignOFSortSet& in
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // start by finding the first dominated Design
    DesignOFSortSet::const_iterator worst(FindDominatedDesign(by, in));
    DesignOFSortSet::const_iterator e(in.end());

    // if we were unable to find a dominated Design, get out.
    if(worst == e) return e;

    // prepare an iterator separate from worst to use.  We will
    // use worst only to store the worst with each test.
    DesignOFSortSet::const_iterator it(worst);

    // now, we need only look beyond worst and update it based
    // on dominance.

    // store the objective function infos for repeated
    // use in the domination compare function
    const ObjectiveFunctionInfoVector& infos =
        by.GetDesignTarget().GetObjectiveFunctionInfos();

    // iterate the range [it, e) where "it" is one past worst
    // to begin with.
    for(++it; it!=e; ++it)
        if(DominationCompare(**worst, **it, infos) == -1) worst = it;

    // return the worst one found.
    return worst;

} // MultiObjectiveStatistician::FindMostDominated

std::size_t
MultiObjectiveStatistician::CountNumDominatedBy(
    const Design& des,
    const DesignOFSortSet& in,
    DesignOFSortSet::const_iterator where,
    bool hasInfeasible,
    int cutoff
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // prepare a return value.
    std::size_t ret = 0;

    // if "des" is not feasible, we must compare it to all other infeasible
    // Designs but since it is cheap to compare a feasible and infeasible for
    // dominance we will just compare them all.
    if(!des.IsFeasible())
    {
        // use "CountDesigns" to do the counting
        return DesignStatistician::CountDesigns(
            in.begin(), in.end(), DominatesPredicate(des), cutoff
            );
    }

    // Otherwise, we need only check those that would be after "des"
    // and count the infeasible in front of "des".  So find the location
    // for "des"
    DesignOFSortSet::const_iterator loc = (where == in.begin()) ?
        in.upper_bound(const_cast<Design*>(&des)) :
        where;

    // count the infeasible in front of des if there are any
    if(hasInfeasible)
        ret += DesignStatistician::CountDesigns(
                in.begin(), loc,
                Design::BoolFuncRetValPred<Design::IsFeasiblePred>(
                    Design::IsFeasiblePred(), false
                    ),
                cutoff
                );

    // now count those that "des" dominates beyond where recall that a feasible
    // Design can only dominate Designs closer to the end of the list with the
    // exception of the infeasible.
    ret += DesignStatistician::CountDesigns(
        loc, in.end(), DominatesPredicate(des), cutoff
        );

    // return the count.
    return ret;

} // MultiObjectiveStatistician::CountNumDominatedBy

std::size_t
MultiObjectiveStatistician::CountNumDominating(
    const Design& des,
    const DesignOFSortSet& in,
    DesignOFSortSet::const_iterator where,
    int cutoff
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // if "des" is not feasible, we must compare it to all other infeasible
    // Designs but since it is cheap to compare a feasible and infeasible
    // for dominance we will just compare them all.
    DesignOFSortSet::const_iterator loc = !des.IsFeasible() ? in.end() :
                                    (where != in.end()) ? where :
                                    in.lower_bound(const_cast<Design*>(&des));

    // now count those that dominate "des" before "where" recall that only a
    // Design closer to the beginning of the container can dominate a feasible
    // Design
    return loc == in.begin() ? 0 : DesignStatistician::CountDesigns(
        in.begin(), loc, DominatedPredicate(des), cutoff
        );

} // MultiObjectiveStatistician::CountNumDominating

DesignCountMap
MultiObjectiveStatistician::ComputeLayers(
    const DesignOFSortSet& designs
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // if designs is empty, there is nothing to do.
    if(designs.empty()) return DesignCountMap();

    // we will do this by repeatedly separating the non-dominated
    // designs from a copy of the passed in deque and updating
    // the returned map.

    // make a temporary copy of the passed in deque that will eventually
    // contain only the feasible.
    DesignOFSortSet feasible;
    DesignOFSortSet infeasible;

	for(DesignOFSortSet::const_iterator it(designs.begin());
		it!=designs.end(); ++it)
			(*it)->IsFeasible() ?
			feasible.insert(*it) : infeasible.insert(*it);

    // prepare the layer value to tag Designs with.
    eddy::utilities::uint64_t layer = 0;

    // prepare our return object.
    DesignCountMap ret(designs.size());

    // prepare a pointer to the collection we are currently considering
    // which will initially be the feasible.
    DesignOFSortSet* currSet = &feasible;

    // do the following for each collection.
    for(eddy::utilities::uint64_t ct = 0; ct<2; ++ct)
    {
        // while the current deque is not empty,
        // separate the non-dominated and label them.
        while(!currSet->empty())
        {
            // Separate the non-dominated from temp
            DesignOFSortSet nonDom(SeparateNonDominated(*currSet));

            // iterate nonDom and insert the layer for each Design.
            for(DesignOFSortSet::const_iterator it(nonDom.begin());
                it!=nonDom.end(); ++it) ret.AddValue(*it, layer);

            // increment the layer
            ++layer;
        }

        // now change to doing the infeasible
        currSet = &infeasible;
    }

    return ret;

} // MultiObjectiveStatistician::ComputeLayers

bool
MultiObjectiveStatistician::IsExtremeDesign(
    const JEGA::Utilities::Design& des,
    const eddy::utilities::DoubleExtremes& paretoExtremes
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // If des has the max or min value for at least all but 1 dimension,
    // it is an extreme design.
    size_t numOutDims = 0;

    const DesignTarget& target = des.GetDesignTarget();
    const size_t nof = target.GetNOF();

    JEGAIFLOG_CF_IT_G_F(nof != paretoExtremes.size(),
        MultiObjectiveStatistician,
        ostream_entry(lfatal(), "Multi-objective Statistician: Extremes "
            "contain record of ") << paretoExtremes.size()
             << " objectives for an " << nof << " objective problem."
        )

    for(DoubleExtremes::size_type i=0; i<nof; ++i)
        if(!paretoExtremes.equals_max_or_min(i, des.GetObjective(i)))
            if((++numOutDims) > 1) return false;

    return true;
}

DesignCountMap
MultiObjectiveStatistician::ComputeDominatedByCounts(
    const DesignOFSortSet& designs,
    int cutoff
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // if designs is empty, there is nothign to do.
    if(designs.empty()) return DesignCountMap();

    // start by breaking "designs" into feasible and infeasible
    DesignOFSortSet feasible;
    DesignOFSortSet infeasible;

	for(DesignOFSortSet::const_iterator it(designs.begin());
		it!=designs.end(); ++it)
			(*it)->IsFeasible() ?
			feasible.insert(*it) : infeasible.insert(*it);

    // Prepare our return object containing our counts.
    DesignCountMap ret(designs.size());
    ret.SuspendStatistics();

    // if the feasible set is empty, skip it.
    if(!feasible.empty())
    {
        // now add in the count of dominating designs to each.
        for(DesignOFSortSet::const_iterator iit(feasible.begin());
            iit!=feasible.end(); ++iit) ret.AddToValue(
                *iit, CountNumDominating(**iit, feasible, iit, cutoff)
                );
    }

    // prepare to now do the infeasible.  if there are none, skip it.
    if(!infeasible.empty())
    {
        // The initial penalty is the number of feasible.  Feasible always
        // dominate infeasible.
        // Add in the count of dominating designs to each.
        for(DesignOFSortSet::const_iterator iit(infeasible.begin());
            iit!=infeasible.end(); ++iit) ret.AddToValue(
                *iit,
                feasible.size() +
                CountNumDominating(**iit, infeasible, iit, cutoff)
                );
    }

    ret.ResumeStatistics(true);
    return ret;

}

DesignCountMap
MultiObjectiveStatistician::ComputeDominatingCounts(
    const DesignOFSortSet& designs,
    int cutoff
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // if designs is empty, there is nothign to do.
    if(designs.empty()) return DesignCountMap();

    // start by breaking "designs" into feasible and infeasible
    DesignOFSortSet feasible;
    DesignOFSortSet infeasible;

	for(DesignOFSortSet::const_iterator it(designs.begin());
		it!=designs.end(); ++it)
			(*it)->IsFeasible() ?
			feasible.insert(*it) : infeasible.insert(*it);

    // Prepare our return object containing our counts.
    DesignCountMap ret(designs.size());
    ret.SuspendStatistics();

    // if the current set is empty, skip it.
    if(!feasible.empty())
    {
        // Prepare to reward the feasible with the number of infeasible.
        const DesignOFSortSet::size_type reward = infeasible.size();

        // prepare to iterate the current set.
        const DesignOFSortSet::const_iterator e(feasible.end());

        // now add in the count of dominating designs to each.
        for(DesignOFSortSet::const_iterator iit(feasible.begin());
            iit!=e; ++iit) ret.AddToValue(
                *iit, reward + CountNumDominatedBy(
                    **iit, feasible, iit, false, cutoff
                    )
                );
    }

    if(!infeasible.empty())
    {
        // prepare to iterate the infeasible.
        const DesignOFSortSet::const_iterator e(infeasible.end());

        // now add in the count of dominating designs to each.
        for(DesignOFSortSet::const_iterator iit(infeasible.begin());
            iit!=e; ++iit) ret.AddToValue(
                *iit, CountNumDominatedBy(
                    **iit, infeasible, iit, true, cutoff
                    )
                );
    }

    ret.ResumeStatistics(true);
    return ret;
}

DesignDoubleCountMap
MultiObjectiveStatistician::ComputeDominationCounts(
    const DesignOFSortSet& designs
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // if designs is empty, there is nothign to do.
    if(designs.empty()) return DesignDoubleCountMap();

    // start by breaking "designs" into feasible and infeasible
    DesignOFSortSet feasible;
    DesignOFSortSet infeasible;

	for(DesignOFSortSet::const_iterator it(designs.begin());
		it!=designs.end(); ++it)
			(*it)->IsFeasible() ?
			feasible.insert(*it) : infeasible.insert(*it);

    // Prepare our return object containing our counts.
    DesignDoubleCountMap ret(designs.size());

    // Get the objective function info's for repeated use.
    const ObjectiveFunctionInfoVector& infos =
        designs.front()->GetDesignTarget().GetObjectiveFunctionInfos();

    if(!feasible.empty())
    {
        // prepare to iterate the current set.
        const DesignOFSortSet::const_iterator e(feasible.end());

        // for each design, see who dominates it and who it dominates.
        for(DesignOFSortSet::const_iterator iit(feasible.begin());
            iit!=e; ++iit)
        {
            DesignDoubleCountMap::value_type numIDom = infeasible.size();

            DesignOFSortSet::const_iterator jit(iit);
            for(++jit; jit!=e; ++jit)
            {
                // Remember that j cannot dominate i b/c we are in the feasible
                // set and the designs are ordered.
                int domResult = DominationCompare(**iit, **jit, infos);
                EDDY_ASSERT(domResult < 1);

                // if iit dominates, increment its domct and jits dombyct.
                if(domResult == -1)
                {
                    ++numIDom;
                    ret.AddToSecondValue(*jit, 1);
                }
            }

            if(numIDom > 0) ret.AddToFirstValue(*iit, numIDom);
        }
    }

    if(!infeasible.empty())
    {
        const DesignOFSortSet::size_type penalty = feasible.size();

        // prepare to iterate the current set.
        const DesignOFSortSet::const_iterator e(infeasible.end());

        // for each design, see who dominates it and who it dominates.
        for(DesignOFSortSet::const_iterator iit(infeasible.begin());
            iit!=e; ++iit)
        {
            DesignDoubleCountMap::value_type numIDom = 0;
            DesignDoubleCountMap::value_type numIDomBy = feasible.size();

            DesignOFSortSet::const_iterator jit(iit);
            for(++jit; jit!=e; ++jit)
            {
                int domResult = DominationCompare(**iit, **jit, infos);

                // if iit dominates, increment its domct and jits dombyct.
                if(domResult == -1)
                {
                    ++numIDom;
                    ret.AddToSecondValue(*jit, 1);
                }
                else if(domResult == 1)
                {
                    ret.AddToFirstValue(*jit, 1);
                    ++numIDomBy;
                }
            }

            if(numIDom > 0) ret.AddToFirstValue(*iit, numIDom);
            if(numIDomBy > 0) ret.AddToSecondValue(*iit, numIDomBy);
        }
    }

    return ret;

} // MultiObjectiveStatistician::ComputeDominationCounts



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
File Scope Utility Class Implementations
================================================================================
*/
DominationPredicate::DominationPredicate(
    const Design& des,
    int value
    ) :
        _des(des),
        _infos(des.GetDesignTarget().GetObjectiveFunctionInfos()),
        _value(value)
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
