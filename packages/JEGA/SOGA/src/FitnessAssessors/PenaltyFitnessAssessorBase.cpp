/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class PenaltyFitnessAssessorBase.

    NOTES:

        See notes of PenaltyFitnessAssessorBase.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jul 29 10:46:50 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the PenaltyFitnessAssessorBase class.
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
#include <../SOGA/include/FitnessAssessors/PenaltyFitnessAssessorBase.hpp>

/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA;
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
const double PenaltyFitnessAssessorBase::DEFAULT_MULTIPLIER(1.0);







/*
================================================================================
Mutators
================================================================================
*/
void
PenaltyFitnessAssessorBase::SetMultiplier(
    double multiplier
    )
{
    EDDY_FUNC_DEBUGSCOPE

    this->_multiplier = multiplier;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": Penalty mutliplier now = "
            ) << this->_multiplier
        )
}


void
PenaltyFitnessAssessorBase::SetWeights(
    const DoubleVector& weights
    )
{
    EDDY_FUNC_DEBUGSCOPE
    const std::size_t nof = this->GetDesignTarget().GetNOF();

    // begin by adopting whatever came in as the new weights.
    this->_weights = weights;

    // now check them for usability and repair if necessary/possible.
    if(weights.size() != nof)
    {
        JEGAIFLOG_CF_II(nof != 1, this->GetLogger(), lquiet(), this,
            ostream_entry(lquiet(), this->GetName() + ": Expected ") << nof
                << " weights.  Received " << weights.size() << "."
                )

        if(weights.size() > nof)
        {
            JEGALOG_II(this->GetLogger(), lquiet(), this,
                text_entry(
                    lquiet(), this->GetName() + ": Truncating extra weights."
                    )
                )
            this->_weights.resize(nof);
        }
        else
        {
            size_t numMissing = static_cast<size_t>(nof-weights.size());
            double currsum = accumulate(weights.begin(), weights.end(), 0.0);
            double fillVal =
                (currsum < 1.0) ? (1.0-currsum) / numMissing : 1.0;

            JEGALOG_II(this->GetLogger(), lquiet(), this,
                ostream_entry(lquiet(),
                    this->GetName() + ": Assigning missing weights to ")
                    << fillVal << "."
                )
            this->_weights.insert(this->_weights.end(), numMissing, fillVal);
        }
    }
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
PenaltyFitnessAssessorBase::AssessFitness(
    const DesignGroupVector& groups
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(ldebug(), this->GetName() + ": Assessing fitness.")
        )

    // Get a set of all weighted sums.
    WeightedSumMap wSums(SingleObjectiveStatistician::ComputeWeightedSums(
        groups, this->GetWeights()
        ));

    // get the deserved penalty of each of our designs
    DesignDoubleMap pens(this->ApplyPenalties(groups));

    // prepare our returned fitness record.
    FitnessRecord* ret = new FitnessRecord(wSums.size());

    const WeightedSumMap::const_iterator e(wSums.end());
    for(WeightedSumMap::const_iterator it=wSums.begin(); it!=e; ++it)
    {
        const Design* des = (*it).first;

        // The fitness will be the sum of weighted sum and penalty.
        // The result is negated so that higher fitness is better.
        ret->AddFitness(des, -(wSums.GetWeightedSum(des) + pens.GetValue(des)));
    }

    JEGALOG_II(this->GetLogger(), lquiet(), this,
        ostream_entry(lverbose(), this->GetName() + ": maximum encountered "
            "fitness = ") << ret->GetMaxFitness() << '.'
        )

    return ret;
}

bool
PenaltyFitnessAssessorBase::PollForParameters(
    const JEGA::Utilities::ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    bool success = ParameterExtractor::GetDoubleFromDB(
        db, "method.constraint_penalty", this->_multiplier
        );

    // If we did not find the crossover rate, warn about it and use the default
    // value.  Note that if !success, then _multiplier has not been altered.
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The penalty multiplier "
            "was not found in the parameter database.  Using the current value "
            "of ") << this->_multiplier
        )

    this->SetMultiplier(this->_multiplier);

    JEGA::DoubleVector tweights;

    success = ParameterExtractor::GetDoubleVectorFromDB(
        db, "responses.multi_objective_weights", tweights
        );

    // If we did not find the weights, warn about it and use the default
    // values.  Note that if !success, then tweights has not been altered.
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        text_entry(lverbose(), this->GetName() + ": The objective weights were "
            "not found in the parameter database.  Using default values.")
        )

    this->SetWeights(tweights);

    return true;
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



PenaltyFitnessAssessorBase::PenaltyFitnessAssessorBase(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmFitnessAssessor(algorithm),
        _multiplier(DEFAULT_MULTIPLIER)
{
    EDDY_FUNC_DEBUGSCOPE
}

PenaltyFitnessAssessorBase::PenaltyFitnessAssessorBase(
    const PenaltyFitnessAssessorBase& copy
    ) :
        GeneticAlgorithmFitnessAssessor(copy),
        _multiplier(copy._multiplier)
{
    EDDY_FUNC_DEBUGSCOPE
}

PenaltyFitnessAssessorBase::PenaltyFitnessAssessorBase(
    const PenaltyFitnessAssessorBase& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmFitnessAssessor(copy, algorithm),
        _multiplier(copy._multiplier)
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

