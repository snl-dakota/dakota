/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implemenation of class SingleObjectiveStatistician.

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
 * \brief Contains the implementation of the SingleObjectiveStatistician class.
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
#include <../Utilities/include/WeightedSumMap.hpp>
#include <../Utilities/include/DesignMultiSet.hpp>
#include <../Utilities/include/DesignGroupVector.hpp>
#include <../Utilities/include/SingleObjectiveStatistician.hpp>


/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA;
using namespace JEGA::Logging;





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
std::pair<double, std::vector<const Design*> >
SingleObjectiveStatistician::FindMinSumDesigns(
    const WeightedSumMap& wsm
    )
{
    EDDY_FUNC_DEBUGSCOPE

    double minSum = DBL_MAX;
    vector<const Design*> bests;

    const WeightedSumMap::const_iterator e(wsm.end());
    for(WeightedSumMap::const_iterator it(wsm.begin()); it!=e; ++it)
    {
        const WeightedSumMap::const_iterator::value_type& item = *it;
        if(item.second < minSum)
        {
            minSum = item.second;
            bests.assign(1, item.first);
        }
        else if(minSum == item.second) bests.push_back(item.first);
    }


    return make_pair(minSum, bests);
}


double
SingleObjectiveStatistician::ComputeWeightedSum(
    const Design& des,
    const JEGA::DoubleVector& weights
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(des.IsEvaluated());
    EDDY_ASSERT(!des.IsIllconditioned());

    const DesignTarget& target = des.GetDesignTarget();

    const ObjectiveFunctionInfoVector& infos =
        target.GetObjectiveFunctionInfos();
    size_t nof = infos.size();

    EDDY_ASSERT(weights.size() == nof);

    double sum = 0.0;
    for(size_t i=0; i<nof; ++i)
        sum += weights[i] * infos[i]->WhichForMinimization(des);

    return sum;

} // SingleObjectiveStatistician::ComputeWeightedSum

WeightedSumMap
SingleObjectiveStatistician::ComputeWeightedSums(
    const DesignGroup& designs,
    const JEGA::DoubleVector& weights
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // create and return a weighted sum map of all Designs.
    WeightedSumMap ret(weights, designs.SizeDV());

    const DesignDVSortSet::const_iterator e(designs.EndDV());
    for(DesignDVSortSet::const_iterator it(designs.BeginDV()); it!=e; ++it)
        ret.AddWeightedSum(*it, ComputeWeightedSum(**it, weights));

    return ret;
}

WeightedSumMap
SingleObjectiveStatistician::ComputeWeightedSums(
    const DesignGroupVector& designs,
    const JEGA::DoubleVector& weights
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // create and return a weighted sum map of all Designs.
    WeightedSumMap ret(weights, designs.GetTotalDesignCount());

    for(size_t i=0; i<designs.size(); ++i)
    {
        const DesignDVSortSet::const_iterator e(designs[i]->EndDV());
        for(DesignDVSortSet::const_iterator jt(designs[i]->BeginDV());
            jt!=e; ++jt)
                ret.AddWeightedSum(*jt, ComputeWeightedSum(**jt, weights));
    }

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
File Scope Utility Class Implementations
================================================================================
*/







/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA
