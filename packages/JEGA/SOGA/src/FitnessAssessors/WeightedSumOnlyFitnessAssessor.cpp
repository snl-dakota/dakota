/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implemenation of class WeightedSumOnlyFitnessAssessor.

    NOTES:

        See notes of WeightedSumOnlyFitnessAssessor.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Sep 17 08:18:58 2008 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the WeightedSumOnlyFitnessAssessor
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
#include <../SOGA/include/SOGA.hpp>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/WeightedSumMap.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>
#include <../Utilities/include/DesignStatistician.hpp>
#include <../Utilities/include/SingleObjectiveStatistician.hpp>
#include <../SOGA/include/FitnessAssessors/WeightedSumOnlyFitnessAssessor.hpp>

/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Logging;
using namespace JEGA::Utilities;







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
WeightedSumOnlyFitnessAssessor::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("weighted_sum_only");
    return ret;
}

const string&
WeightedSumOnlyFitnessAssessor::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This fitness assessor specializes the PenaltyFitnessAssessor "
        "but does not alter the weighted sum values."
        );
    return ret;
}

GeneticAlgorithmOperator*
WeightedSumOnlyFitnessAssessor::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new WeightedSumOnlyFitnessAssessor(algorithm);
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

const FitnessRecord*
WeightedSumOnlyFitnessAssessor::AssessFitness(
    const DesignGroupVector& groups
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(ldebug(), this->GetName() + ": Assessing fitness.")
        )

    // Get a set of all weighted sums.
    WeightedSumMap wSums(
        SingleObjectiveStatistician::ComputeWeightedSums(
            groups, this->GetWeights()
            )
        );

    // prepare our returned fitness record.
    FitnessRecord* ret = new FitnessRecord(wSums.size());

    WeightedSumMap::const_iterator e(wSums.end());
    for(WeightedSumMap::const_iterator it=wSums.begin(); it!=e; ++it)
    {
        const Design* des = (*it).first;

        // The fitness will be the weighted sum only.
        // The result is negated so that higher fitness is better.
        ret->AddFitness(des, -wSums.GetWeightedSum(des));
    }

    return ret;
}

DesignDoubleMap
WeightedSumOnlyFitnessAssessor::ApplyPenalties(
    const DesignGroupVector& groups
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // This should never get called anyway.
    return DesignDoubleMap();
}

string
WeightedSumOnlyFitnessAssessor::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return WeightedSumOnlyFitnessAssessor::Name();
}

string
WeightedSumOnlyFitnessAssessor::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return WeightedSumOnlyFitnessAssessor::Description();
}

GeneticAlgorithmOperator*
WeightedSumOnlyFitnessAssessor::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new WeightedSumOnlyFitnessAssessor(*this, algorithm);
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


WeightedSumOnlyFitnessAssessor::WeightedSumOnlyFitnessAssessor(
    GeneticAlgorithm& algorithm
    ) :
        PenaltyFitnessAssessorBase(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

WeightedSumOnlyFitnessAssessor::WeightedSumOnlyFitnessAssessor(
    const WeightedSumOnlyFitnessAssessor& copy
    ) :
        PenaltyFitnessAssessorBase(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

WeightedSumOnlyFitnessAssessor::WeightedSumOnlyFitnessAssessor(
    const WeightedSumOnlyFitnessAssessor& copy,
    GeneticAlgorithm& algorithm
    ) :
        PenaltyFitnessAssessorBase(copy, algorithm)
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
