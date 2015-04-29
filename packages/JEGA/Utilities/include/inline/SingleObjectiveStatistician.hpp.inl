/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class SingleObjectiveStatistician.

    NOTES:

        See notes of SingleObjectiveStatistician.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 22 08:02:43 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the SingleObjectiveStatistician class.
 */


/*
================================================================================
Includes
================================================================================
*/
#include <../Utilities/include/WeightedSumMap.hpp>
#include <../Utilities/include/DesignStatistician.hpp>






/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Utilities {







/*
================================================================================
Inline Mutators
================================================================================
*/







/*
================================================================================
Inline Accessors
================================================================================
*/






/*
================================================================================
Inline Public Methods
================================================================================
*/


template <typename DesignContainer>
std::pair<double, std::vector<typename DesignContainer::const_iterator> >
SingleObjectiveStatistician::FindMinSumDesigns(
    const DesignContainer& cont,
    const JEGA::DoubleVector& weights
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // prepare to store the lowest value and associated Designs
    double minSum = std::numeric_limits<double>::max();
    std::vector<typename DesignContainer::const_iterator> bests;

    // prepare to iterate the cont
    typename DesignContainer::const_iterator it(cont.begin());
    for(; it!=cont.end(); ++it)
    {
        // ignore designs that are not evalauted.
        if(!(*it)->IsEvaluated()) continue;

        double cws = ComputeWeightedSum(**it, weights);
        if(cws < minSum)
        {
            minSum = cws;
            bests.assign(1, it);
        }
        else if(cws == minSum) bests.push_back(it);
    }

    return std::make_pair(minSum, bests);

}

template <typename DesignContainer>
std::pair<double, std::vector<typename DesignContainer::const_iterator> >
SingleObjectiveStatistician::FindMinSumFeasibleDesigns(
    const DesignContainer& cont,
    const JEGA::DoubleVector& weights
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // prepare to store the lowest value and associated Design
    double minSum = std::numeric_limits<double>::max();
    std::vector<typename DesignContainer::const_iterator> bests;

    // prepare to iterate the cont
    typename DesignContainer::const_iterator it(cont.begin());

    for(; it!=cont.end(); ++it)
    {
        // ignore designs that are not evalauted.
        if(!(*it)->IsFeasible()) continue;

        double cws = ComputeWeightedSum(**it, weights);
        if(cws < minSum)
        {
            minSum = cws;
            bests.assign(1, it);
        }
        else if(cws == minSum) bests.push_back(it);
    }

    return std::make_pair(minSum, bests);
}

template <typename DesignContainer>
std::pair<double, std::vector<typename DesignContainer::const_iterator> >
SingleObjectiveStatistician::FindMinSumMinViolateDesigns(
    const DesignContainer& cont,
    const JEGA::DoubleVector& weights
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // prepare to store the lowest values and associated Designs
    double minViol = std::numeric_limits<double>::max();
    double minSum = std::numeric_limits<double>::max();

    std::vector<typename DesignContainer::const_iterator> bests;

    // prepare to iterate the cont.  Give priority to minimum violation and
    // secondary consideration to the weighted sum.
    for(typename DesignContainer::const_iterator it(cont.begin());
        it!=cont.end(); ++it)
    {
        // ignore designs that are not evaluated.
        if(!(*it)->IsEvaluated()) continue;

        double viol = DesignStatistician::ComputeTotalPercentageViolation(**it);
        if(viol < minViol)
        {
            // The weighed sum of the new best design is the new best weighted
            // sum as well.
            double cws = ComputeWeightedSum(**it, weights);
            minViol = viol;
            minSum = cws;
            bests.assign(1, it);
        }
        else if(viol == minViol)
        {
            // We have one that is equally good in terms of constraint
            // violation.  See if it is as good or better than the current
            // solutions in terms of weighted sum.
            double cws = ComputeWeightedSum(**it, weights);
            if(cws < minSum)
            {
                minSum = cws;
                bests.assign(1, it);
            }
            else if(cws == minSum)
            {
                bests.push_back(it);
            }
        }
    }

    return std::make_pair(minSum, bests);
}

template <typename DesignContainer>
WeightedSumMap
SingleObjectiveStatistician::ComputeWeightedSums(
    const DesignContainer& designs,
    const JEGA::DoubleVector& weights
    )
{
    // create and return a weighted sum map of all Designs.
    WeightedSumMap ret(weights, designs.size());

    for(typename DesignContainer::const_iterator it(designs.begin());
        it!=designs.end(); ++it)
            ret.AddWeightedSum(*it, ComputeWeightedSum(**it, weights));

    return ret;
}





/*
================================================================================
Inline Subclass Visible Methods
================================================================================
*/








/*
================================================================================
Inline Private Methods
================================================================================
*/








/*
================================================================================
Inline Structors
================================================================================
*/








/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA
