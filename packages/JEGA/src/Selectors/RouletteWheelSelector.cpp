/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class RouletteWheelSelector.

    NOTES:

        See notes of RouletteWheelSelector.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Jun 23 16:10:28 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the RouletteWheelSelector class.
 */





/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <cfloat>
#include <limits>
#include <algorithm>
#include <FitnessRecord.hpp>
#include <GeneticAlgorithm.hpp>
#include <../Utilities/include/Logging.hpp>
#include <Selectors/RouletteWheelSelector.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignGroupVector.hpp>
#include <utilities/include/RandomNumberGenerator.hpp>



/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA;
using namespace JEGA::Utilities;
using namespace JEGA::Logging;
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
const size_t RouletteWheelSelector::SEL_ATT = 7;






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
RouletteWheelSelector::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("roulette_wheel");
    return ret;
}

const string&
RouletteWheelSelector::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret(
        "This selector implements the common roulette wheel selection "
        "scheme.  Conceptually, each Design is allotted a portion of a "
        "wheel proportional to its fitness relative to the fitnesses of the "
        "other Designs.  Then portions of the wheel are chosen at random and "
        "the Design occupying those portions are duplicated into the next "
        "population.  Those Designs allotted larger portions of the wheel "
        "are more likely to be selected potentially many times."
        );
    return ret;
}

GeneticAlgorithmOperator*
RouletteWheelSelector::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new RouletteWheelSelector(algorithm);
}






/*
================================================================================
Subclass Visible Methods
================================================================================
*/
void
RouletteWheelSelector::ClearSelectionAttributes(
    const DesignGroupVector& from
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // prepare to iterate the groups.
    DesignGroupVector::const_iterator git(from.begin());

    for(; git!=from.end(); ++git)
    {
        // prepare to iterate the Designs of this group
        DesignOFSortSet::const_iterator dit((*git)->BeginOF());
        const DesignOFSortSet::const_iterator de((*git)->EndOF());
        for(; dit!=de; ++dit)
            // clear the SEL_ATT while we're here
            (*dit)->ModifyAttribute(SEL_ATT, false);
    }
}

void
RouletteWheelSelector::GenerateCumulativeProbabilities(
    const DesignGroupVector& from,
    const FitnessRecord& fitnesses,
    CumulativeProbabilityMap& into,
    bool skipSelected
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // if there are no Designs, get out.
    if(from.GetTotalDesignCount() == 0) { into.clear(); return; }

    double minfit = std::numeric_limits<double>::max();
    double maxfit = -std::numeric_limits<double>::max();
    double fitsum = 0.0;
    size_t desCt = 0;

    // prepare to iterate the groups multiple times.
    for(DesignGroupVector::const_iterator git(from.begin());
        git!=from.end(); ++git)
    {
        const DesignOFSortSet::const_iterator de((*git)->EndOF());
        for(DesignOFSortSet::const_iterator dit((*git)->BeginOF());
            dit!=de; ++dit)
        {
            if(skipSelected && (*dit)->HasAttribute(SEL_ATT)) continue;
            double currFit = fitnesses.GetFitness(**dit);
            minfit = Math::Min(currFit, minfit);
            maxfit = Math::Max(currFit, maxfit);
            fitsum += currFit;
            desCt++;
        }
    }

    // prepare to iterate the groups multiple times.
    DesignGroupVector::const_iterator git(from.begin());

    // First Scenario - minfit == maxfit
    // if the min fitness is equal to the max fitness, then
    // all designs have the same fitness and thus get an equal
    // slice of the pie.  So do it that way and get out.
    if(minfit == maxfit)
    {
        double probinc = 1.0 / desCt;
        double currprob = probinc;

        for(git=from.begin(); git!=from.end(); ++git)
        {
            if((*git)->SizeOF() == 0) continue;
            const DesignOFSortSet::const_iterator  de((*git)->EndOF());
            for(DesignOFSortSet::const_iterator dit((*git)->BeginOF());
                dit!=de; ++dit)
            {
                if(skipSelected && (*dit)->HasAttribute(SEL_ATT)) continue;
                into.insert(
                    CumulativeProbabilityMap::value_type(currprob, *dit)
                    );
                currprob += probinc;
            }
        }
    }

    // Second Scenario - minfit is negative
    // we are only concerned with using the minfit if it is negative.
    // if not, we don't need to adjust the fitnesses at all.
    // so if it is negative, bump it up to 0.  If it is not, leave it alone.
    // adjustment will be subtracted from each fitness value.
    double adjustment = Math::Min(minfit, 0.0);

    // now correct the fitness sum.  It must be adjusted by the adjustment.
    fitsum -= desCt * adjustment;

    // if the sum of fitnesses is 0, it likely means that all fitness values
    // are equal and negative or 0.  This is allowed but must be handled since
    // it could cause division by 0.
    double commonprob = fitsum == 0.0 ? 1.0 / desCt : fitsum;

    // compute the cumulative probability for each member.
    double currprob = 0.0;

    // iterate the groups.
    for(git=from.begin(); git!=from.end(); ++git)
    {
        EDDY_ASSERT(*git != 0x0);

        // prepare to iterate the Designs of this group
        DesignOFSortSet::const_iterator dit((*git)->BeginOF());
        const DesignOFSortSet::const_iterator de((*git)->EndOF());

        // iterate the Designs and compute the cumulative probabilities.
        for(; dit!=de; ++dit)
        {
            if(skipSelected && (*dit)->HasAttribute(SEL_ATT)) continue;

            double currFit = fitnesses.GetFitness(**dit);

            if(currFit != -DBL_MAX)
            {
                // compute the cumulative probability for this Design.
                currprob += fitsum == 0.0 ?
                    commonprob : (currFit - adjustment) / fitsum;

                // store it in the vector of probabilities.
                into.insert(
                    CumulativeProbabilityMap::value_type(currprob, *dit)
                    );
            }

            JEGA_LOGGING_IF_ON(else)
                JEGALOG_II(this->GetLogger(), lquiet(), this,
                    text_entry(lquiet(), this->GetName() +
                        ": Fitness not found for supplied design.")
                    )
        }
    }

    // make the last cumprob slightly greater than one so that
    // upper_bound will not return cumprob.end() (since U01
    // cannot be larger than 1.0).
    //
    // This does not give the last Design any sort of advantage
    // since U01 will never exist in the enlarged region.  We
    // could just as well make this number 10,000,000.
    if(!into.empty())
    {
        Design* last = (*(--into.end())).second;
        into.erase(--into.end());
        into.insert(CumulativeProbabilityMap::value_type(1.03125, last));
    }
}

void
RouletteWheelSelector::RecordSelectedDesigns(
    const DesignGroupVector& from,
    DesignGroup& into
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // prepare to iterate over the passed in groups.
    DesignGroupVector::const_iterator git(from.begin());

    for(; git!=from.end(); ++git)
    {
        DesignGroup& dg = **git;

        // iterate over the Designs.
        const DesignOFSortSet::const_iterator e(dg.EndOF());
        for(DesignOFSortSet::iterator dit(dg.BeginOF()); dit!=e; )
        {
            // move any designs with the selection attribute into
            // the "into" group.  Leave the rest alone.
            Design* des = *dit;
            if(des->HasAttribute(SEL_ATT))
            {
                into.Insert(des);
                des->ModifyAttribute(SEL_ATT, false);
                dit = dg.EraseRetOF(dit);
            }
            else ++dit;
        }
    }
}

/*
================================================================================
Subclass Overridable Methods
================================================================================
*/


string
RouletteWheelSelector::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return RouletteWheelSelector::Name();
}

string
RouletteWheelSelector::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return RouletteWheelSelector::Description();
}

GeneticAlgorithmOperator*
RouletteWheelSelector::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new RouletteWheelSelector(*this, algorithm);
}


void
RouletteWheelSelector::Select(
    DesignGroupVector& from,
    DesignGroup& into,
    size_t count,
    const FitnessRecord& fitnesses
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(ldebug(), this->GetName() + ": Performing selection.")
        )

    // Check for trivial abort condition.
    if(from.empty()) return;

    JEGA_LOGGING_IF_ON(const std::size_t prevSize = into.GetSize();)

    // get the target for information.
    EDDY_DEBUGEXEC(DesignTarget& target = into.GetDesignTarget())

    // prepare to iterate over the passed in groups.
    DesignGroupVector::const_iterator git(from.begin());
    const DesignGroupVector::const_iterator e(from.end());

    // do some asserting if in debug mode
    EDDY_DEBUGEXEC(
        // verify all targets the same.
        for(; git!=from.end(); ++git) {
            EDDY_ASSERT(*git != 0x0);
            EDDY_ASSERT(&(*git)->GetDesignTarget() == &target);
        }
    );

    // create a vector for the cumulative probabilities
    CumulativeProbabilityMap cumprob;

    // get the cumulative probabilities
    this->GenerateCumulativeProbabilities(from, fitnesses, cumprob, false);

    // if cumprob is empty, there were no Designs, lets get out.
    if(cumprob.empty()) return;

    // clear the selection attributes just in case
    this->ClearSelectionAttributes(from);

    // now fill the "into" group with randomly selected members from the groups
    for(size_t i=0; i<count; ++i)
    {
        // find the entry in cumprob associated with U[0,1]
        CumulativeProbabilityMap::const_iterator mem =
            cumprob.upper_bound(RandomNumberGenerator::UniformReal(0.0, 1.0));
        EDDY_ASSERT(mem != cumprob.end());

        JEGAIFLOG_CF_II(mem == cumprob.end(), this->GetLogger(), lquiet(), this,
            text_entry(lquiet(),  this->GetName() +
                ": U[0,1] not bounded within cumulative probability map.")
            )

        // figure out which design that references.
        Design* seldes = (*mem).second;
        JEGAIFLOG_CF_II_F(seldes == 0, this->GetLogger(), this,
            text_entry(lfatal(), this->GetName() +
            ": Null Design returned from cumulative probability map.")
            )

        // If this Design has not yet been chosen, mark it otherwise.
        // We will put it in "into" later after all selections have been
        // made.  That way we can remove it from its current group and
        // not disallow multiple selections.
        if(!seldes->HasAttribute(SEL_ATT))
            seldes->ModifyAttribute(SEL_ATT, true);

        // otherwise, make a copy for the into group.
        else
        {
            // Get a copy from the algorithm.
            Design* newdes = this->GetAlgorithm().GetNewDesign(*seldes);

            // The copy will have SEL_ATT but we don't need it.
            newdes->ModifyAttribute(SEL_ATT, false);

            // Put newdes into "into".
            into.Insert(newdes);
        }
    }

    // Now put in all selected Designs into "into".  (Recall that
    // so far, only duplications of those that were selected
    // more than once are in "into").
    RecordSelectedDesigns(from, into);

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": Performed ")
            << (into.GetSize() - prevSize) << " total selections."
        )
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

RouletteWheelSelector::RouletteWheelSelector(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmSelector(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

RouletteWheelSelector::RouletteWheelSelector(
    const RouletteWheelSelector& copy
    ) :
        GeneticAlgorithmSelector(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

RouletteWheelSelector::RouletteWheelSelector(
    const RouletteWheelSelector& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmSelector(copy, algorithm)
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
