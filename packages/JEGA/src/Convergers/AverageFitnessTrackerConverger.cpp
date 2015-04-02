/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class AverageFitnessTrackerConverger.

    NOTES:

        See notes of AverageFitnessTrackerConverger.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jul 29 09:50:11 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the AverageFitnessTrackerConverger
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
#include <FitnessRecord.hpp>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <Convergers/AverageFitnessTrackerConverger.hpp>







/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Utilities;
using namespace JEGA::Logging;







/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Algorithms {





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



const string&
AverageFitnessTrackerConverger::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("average_fitness_tracker");
    return ret;
}

const string&
AverageFitnessTrackerConverger::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret(
        "This converger returns true if the average observed fitness "
        "value changes by less than the input percent change over the input "
        "number of generations."
        );
    return ret;
}

GeneticAlgorithmOperator*
AverageFitnessTrackerConverger::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new AverageFitnessTrackerConverger(algorithm);
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



double
AverageFitnessTrackerConverger::GetMetricValue(
    const DesignGroup& group,
    const FitnessRecord& fitnesses
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // look for trivial abort conditions.
    if(group.IsEmpty())
    {
        JEGALOG_II(this->GetLogger(), lquiet(), this,
            text_entry(lquiet(), this->GetName() + ": Empty group received for "
                       "fitness assessment.")
            )
        return -DBL_MAX;
    }

    double totalFit = 0.0;
    size_t nFits = 0;

    // need to find out the total fitness value in "group".
    for(DesignOFSortSet::const_iterator it(group.BeginOF());
        it!=group.EndOF(); ++it)
    {
        double currFit = fitnesses.GetFitness(**it);

        if(currFit != -DBL_MAX)
        {
            ++nFits;
            totalFit += currFit;
        }
        JEGA_LOGGING_IF_ON(else)
        {
            JEGALOG_II(this->GetLogger(), lquiet(), this,
                text_entry(lquiet(), this->GetName() + ": No fitness record "
                           "for supplied design.")
                )
        }
    }

    if(nFits == 0)
    {
        JEGALOG_II(this->GetLogger(), lquiet(), this,
            text_entry(lquiet(), this->GetName() +
                ": No fitnesses could be retrieved for testing.")
            )
        return -DBL_MAX;
    }

    double avgFit = totalFit / nFits;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": Average fitness = ")
            << avgFit
        )

    return avgFit;
}

string
AverageFitnessTrackerConverger::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return AverageFitnessTrackerConverger::Name();
}

string
AverageFitnessTrackerConverger::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return AverageFitnessTrackerConverger::Description();
}

GeneticAlgorithmOperator*
AverageFitnessTrackerConverger::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new AverageFitnessTrackerConverger(*this, algorithm);
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





AverageFitnessTrackerConverger::AverageFitnessTrackerConverger(
    GeneticAlgorithm& algorithm
    ) :
        MetricTrackerConvergerBase(algorithm, false)
{
    EDDY_FUNC_DEBUGSCOPE
}

AverageFitnessTrackerConverger::AverageFitnessTrackerConverger(
    const AverageFitnessTrackerConverger& copy
    ) :
        MetricTrackerConvergerBase(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

AverageFitnessTrackerConverger::AverageFitnessTrackerConverger(
    const AverageFitnessTrackerConverger& copy,
    GeneticAlgorithm& algorithm
    ) :
        MetricTrackerConvergerBase(copy, algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}








/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA

