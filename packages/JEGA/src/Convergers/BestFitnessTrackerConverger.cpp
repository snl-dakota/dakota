/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class BestFitnessTrackerConverger.

    NOTES:

        See notes of BestFitnessTrackerConverger.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jul 22 16:10:25 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the BestFitnessTrackerConverger class.
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
#include <utilities/include/Math.hpp>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <Convergers/BestFitnessTrackerConverger.hpp>




/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Logging;
using namespace JEGA::Utilities;
using namespace eddy::utilities;








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
BestFitnessTrackerConverger::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("best_fitness_tracker");
    return ret;
}

const string&
BestFitnessTrackerConverger::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret(
        "This converger returns true if the best observed fitness "
        "value changes by less than the input percent change over the input "
        "number of generations."
        );
    return ret;
}

GeneticAlgorithmOperator*
BestFitnessTrackerConverger::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new BestFitnessTrackerConverger(algorithm);
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
BestFitnessTrackerConverger::GetMetricValue(
    const DesignGroup& group,
    const FitnessRecord& fitnesses
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // look for trivial abort conditions.
    if(group.IsEmpty())
    {
        JEGALOG_II(this->GetLogger(), lquiet(), this,
            text_entry(lquiet(), this->GetName() +
                ": Empty group received for fitness assessment.")
            )
        return DBL_MAX;
    }

    double bestFit = -DBL_MAX;

    for(DesignOFSortSet::const_iterator it(group.BeginOF());
        it!=group.EndOF(); ++it)
    {
        double currFit = fitnesses.GetFitness(**it);

        if(currFit != -DBL_MAX)
            bestFit = Math::Max(bestFit, currFit);

        JEGA_LOGGING_IF_ON(else)
            JEGALOG_II(this->GetLogger(), lquiet(), this,
                text_entry(lquiet(), this->GetName() + ": No fitness record "
                    "for supplied design.")
                )
    }

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": Best fitness = ")
            << bestFit
        )

    return bestFit;
}



string
BestFitnessTrackerConverger::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return BestFitnessTrackerConverger::Name();
}

string
BestFitnessTrackerConverger::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return BestFitnessTrackerConverger::Description();
}

GeneticAlgorithmOperator*
BestFitnessTrackerConverger::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new BestFitnessTrackerConverger(*this, algorithm);
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




BestFitnessTrackerConverger::BestFitnessTrackerConverger(
    GeneticAlgorithm& algorithm
    ) :
        MetricTrackerConvergerBase(algorithm, false)
{
    EDDY_FUNC_DEBUGSCOPE
}

BestFitnessTrackerConverger::BestFitnessTrackerConverger(
    const BestFitnessTrackerConverger& copy
    ) :
        MetricTrackerConvergerBase(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

BestFitnessTrackerConverger::BestFitnessTrackerConverger(
    const BestFitnessTrackerConverger& copy,
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
