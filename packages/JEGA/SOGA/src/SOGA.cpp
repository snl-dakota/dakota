/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class SOGA.

    NOTES:

        See notes of SOGA.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Jun 02 14:26:59 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the SOGA class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <cfloat>
#include <numeric>
#include <FitnessRecord.hpp>
#include <../SOGA/include/SOGA.hpp>
#include <../Utilities/include/Logging.hpp>
#include <GeneticAlgorithmFitnessAssessor.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignGroupVector.hpp>
#include <../Utilities/include/DesignStatistician.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>
#include <../Utilities/include/SingleObjectiveStatistician.hpp>
#include <../SOGA/include/OperatorGroups/SOGAOperatorGroup.hpp>
#include <../SOGA/include/OperatorGroups/FavorFeasibleOperatorGroup.hpp>

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
const bool SOGA::_registered_operator_groups =


    SOGA::RegistryOfOperatorGroups().register_(
        SOGAOperatorGroup::Name(),
        &SOGAOperatorGroup::Instance
        ) &&

    SOGA::RegistryOfOperatorGroups().register_(
        FavorFeasibleOperatorGroup::Name(),
        &FavorFeasibleOperatorGroup::Instance
        ) &&

true;





/*
================================================================================
Mutators
================================================================================
*/


void
SOGA::SetWeights(
    const DoubleVector& weights
    )
{
    EDDY_FUNC_DEBUGSCOPE
    const std::size_t nof = GetDesignTarget().GetNOF();

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
                text_entry(lquiet(), this->GetName() +
                    ": Truncating extra weights.")
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
GeneticAlgorithmOperatorGroupRegistry&
SOGA::RegistryOfOperatorGroups(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static GeneticAlgorithmOperatorGroupRegistry registry;
    return registry;
}

void
SOGA::ReclaimOptimal(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // We are only interested in reclaiming Designs if they are feasible
    // regardless of the current state of the population.

    // Go through all the discarded Designs and re-claim any
    // that are non-dominated but got selected out.
    DesignTarget& target = this->GetDesignTarget();

    // store the discards for repeated use.
    const DesignDVSortSet& discards = target.CheckoutDiscards();

    // separate the feasible from the discards because
    // That's all we're interested in.
    DesignDVSortSet feasible(DesignStatistician::GetFeasible(discards));

    // if there are no feasible, get out.
    if(feasible.empty()) { target.CheckinDiscards(); return; }

    // otherwise, we have to find the best weighted sum in the deque.
    pair<double, vector<DesignDVSortSet::const_iterator> > bests(
        SingleObjectiveStatistician::FindMinSumDesigns(
            feasible, this->GetWeights()
            )
        );

    std::size_t numReclaimed = 0;

    // if there are no feasible in the population, we
    // will absorb the "bests" and get out.
    if(this->GetPopulation().CountFeasible() == 0)
    {
        while(!bests.second.empty())
        {
            // Get the discarded Design out of the discards.
            target.ReclaimDesign(**bests.second.back());

            // put this guy in the population.
            this->GetPopulation().Insert(*bests.second.back());

            // pop him out of the bests vector.
            bests.second.pop_back();

            ++numReclaimed;
        }
    }

    // Otherwise, we will absorb the bests if they are better
    // than the best in the population.
    else
    {
        pair<double, vector<DesignOFSortSet::const_iterator> > pbests(
            SingleObjectiveStatistician::FindMinSumFeasibleDesigns(
                this->GetPopulation().GetOFSortContainer(), this->GetWeights()
                )
            );

        EDDY_ASSERT(!pbests.second.empty());

        if(bests.first <= pbests.first)
        {
            while(!bests.second.empty())
            {
                // put this guy in the population.
                this->GetPopulation().Insert(
                    target.GetNewDesign(**bests.second.back())
                    );

                // pop him out of the bests vector.
                bests.second.pop_back();

                ++numReclaimed;
            }
        }
    }

    // now we can unlock the discards
    target.CheckinDiscards();

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": Relcaimed ")
            << numReclaimed << " optimal designs from the discards."
            )
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
SOGA::PollForParameters(
    const JEGA::Utilities::ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA::DoubleVector tweights;

    bool success = ParameterExtractor::GetDoubleVectorFromDB(
        db, "responses.multi_objective_weights", tweights
        );

    // If we did not find the weights, warn about it and use the default
    // values.  Note that if !success, then tweights has not been altered.
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        text_entry(lverbose(), this->GetName() + ": The objective weights were "
            "not found in the parameter database.  Using default values.")
        )

    this->SetWeights(tweights);

    return this->GeneticAlgorithm::PollForParameters(db);
}

void
SOGA::FlushNonOptimal(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Store the population for repeated use.
    DesignGroup& pop = this->GetPopulation();

    // now flush out all non-optimal designs.
    pair<double, vector<DesignOFSortSet::const_iterator> > bests(
        SingleObjectiveStatistician::FindMinSumMinViolateDesigns(
            pop.GetOFSortContainer(), this->GetWeights()
            )
        );

    // if bests is as big as the population, we have only optimal and we don't
    // need to continue.
    if(bests.second.size() == pop.SizeDV()) return;

    // unset all 7 attributes in all designs.
    for(DesignDVSortSet::const_iterator it(pop.BeginDV());
        it!=pop.EndDV(); ++it) (*it)->ModifyAttribute(7, false);

    // tag all the bests with a marker so we can flush the others.
    for(vector<DesignOFSortSet::const_iterator>::const_iterator it(
            bests.second.begin()
            ); it!=bests.second.end(); ++it)
                (**it)->ModifyAttribute(7, true);

    // now flush all the non-optimal.
    pop.FlushDesigns(7, false);
}

DesignOFSortSet
SOGA::GetCurrentSolution(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    typedef vector<DesignOFSortSet::const_iterator> itofvec;
    typedef vector<DesignDVSortSet::const_iterator> itdvvec;

    // if we've finalized, the current solution is in the population.
    // There is no need to search the discards, etc.
    if(this->IsFinalized()) return this->GetPopulation().GetOFSortContainer();

    // first, figure out the bests of the population.
    pair<double, itofvec> popBests(
        SingleObjectiveStatistician::FindMinSumMinViolateDesigns(
            this->GetPopulation().GetOFSortContainer(), this->GetWeights()
            )
        );

    // now figure out the bests of the discards.
    const DesignDVSortSet& discards =
        this->GetDesignTarget().CheckoutDiscards();

    pair<double, itdvvec> disBests(
        SingleObjectiveStatistician::FindMinSumMinViolateDesigns(
            discards, this->GetWeights()
            )
        );

    DesignOFSortSet ret;

    // now create a solution set from the bests of the bests.
    if(popBests.first <= disBests.first)
        for(itofvec::const_iterator it(popBests.second.begin());
            it!=popBests.second.end(); ++it) ret.insert(**it);

    if(disBests.first <= popBests.first)
        for(itdvvec::const_iterator it(disBests.second.begin());
            it!=disBests.second.end(); ++it) ret.insert(**it);

    // check the discards back in before we exit.
    this->GetDesignTarget().CheckinDiscards();

    return ret;
}


GeneticAlgorithmOperatorGroupRegistry&
SOGA::GetOperatorGroupRegistry(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return RegistryOfOperatorGroups();
}

string
SOGA::GetAlgorithmTypeName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return "soga";
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
SOGA::SOGA(
    DesignTarget& target,
    Logger& logger
    ) :
        GeneticAlgorithm(target, logger),
        _weights(target.GetNOF(), 1.0 / target.GetNOF())
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
