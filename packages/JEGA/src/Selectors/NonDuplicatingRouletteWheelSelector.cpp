/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class NonDuplicatingRouletteWheelSelector.

    NOTES:

        See notes of NonDuplicatingRouletteWheelSelector.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Jun 25 11:33:48 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the
 *        NonDuplicatingRouletteWheelSelector class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <cfloat>
#include <algorithm>
#include <FitnessRecord.hpp>
#include <GeneticAlgorithm.hpp>
#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignGroupVector.hpp>
#include <utilities/include/RandomNumberGenerator.hpp>
#include <Selectors/NonDuplicatingRouletteWheelSelector.hpp>






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
NonDuplicatingRouletteWheelSelector::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("unique_roulette_wheel");
    return ret;
}

const string&
NonDuplicatingRouletteWheelSelector::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret(
        "This selector implements the common roulette wheel selection without "
        "duplication.  This is the same as regular roulette wheel selection "
        "with the exception that a Design may only be selected once.  See the "
        "documentation for the RouletteWheelSelector for a brief description "
        "of roulette wheel selection."
        );
    return ret;
}

GeneticAlgorithmOperator*
NonDuplicatingRouletteWheelSelector::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new NonDuplicatingRouletteWheelSelector(algorithm);
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



string
NonDuplicatingRouletteWheelSelector::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NonDuplicatingRouletteWheelSelector::Name();
}

string
NonDuplicatingRouletteWheelSelector::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NonDuplicatingRouletteWheelSelector::Description();
}

GeneticAlgorithmOperator*
NonDuplicatingRouletteWheelSelector::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new NonDuplicatingRouletteWheelSelector(*this, algorithm);
}


void
NonDuplicatingRouletteWheelSelector::Select(
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

    // if "count" is more than the total number of Designs
    // we will just select everything
    size_t tsize = from.GetTotalDesignCount();

    if(count >= tsize)
    {
        // prepare to iterate the groups.
        DesignGroupVector::const_iterator git(from.begin());

        for(; git!=from.end(); ++git)
        {
            // prepare to iterate the Designs of this group
            DesignOFSortSet::const_iterator e((*git)->EndOF());
            for(DesignOFSortSet::const_iterator it((*git)->BeginOF());
                it!=e; ++it) (*it)->ModifyAttribute(SEL_ATT, true);
        }
        count = tsize;
    }
    else
    {
        // get the target for information.
        EDDY_DEBUGEXEC(DesignTarget& target = this->GetDesignTarget())

        // prepare to iterate over the passed in groups.
        DesignGroupVector::const_iterator git(from.begin());

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

        // prepare to tolerate no more than 10*tsize consecutive failures before
        // re-creating the cumulative probabilities.
        size_t consecFails = 0;

        // now fill the "into" group with randomly selected members from the
        // groups
        for(size_t i=0; i<count; ++i)
        {
            // find the entry in cumprob associated with U[0,1]
            CumulativeProbabilityMap::const_iterator mem =
                cumprob.upper_bound(
                    RandomNumberGenerator::UniformReal(0.0, 1.0)
                    );

            EDDY_ASSERT(mem != cumprob.end());
            JEGAIFLOG_CF_II(mem == cumprob.end(), this->GetLogger(), lquiet(),
                    this, text_entry(lquiet(), "U[0,1] not bounded within "
                               "cumulative probability map.")
                    )

            // Store the currently selected Design.
            Design* seldes = (*mem).second;

            // If this Design has not yet been chosen, mark it otherwise.
            // We will put it in "into" later after all selections have been
            // made.  That way we can remove it from its current group and
            // not disallow multiple selections.
            if(!seldes->HasAttribute(SEL_ATT))
            {
                seldes->ModifyAttribute(SEL_ATT, true);
                consecFails = 0;
            }

            // otherwise, decrement i and continue.
            else { ++consecFails; --i; }

            // if we have brieched the tolerable number of consecutive
            // failures, we will stop now.
            if(consecFails >= (10*tsize))
            {
                JEGALOG_II(this->GetLogger(), lquiet(), this,
                    ostream_entry(lquiet(), this->GetName() + ": Failed to "
                        "make a unique selection in ")
                        << consecFails << " consecutive tries after " << i
                        << " successful selections.  Re-computing cumulative "
                           "probabilities."
                    )

                cumprob.clear();

                this->GenerateCumulativeProbabilities(
                    from, fitnesses, cumprob, true
                    );

                consecFails = 0;
            }
        }
    }

    // Now put in all selected Designs into "into".  (Recall that
    // so far, only duplications of those that were selected
    // more than once are in "into").
    this->RecordSelectedDesigns(from, into);

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": Performed ")
            << (into.GetSize() - prevSize) << " total selections."
        )
}

bool
NonDuplicatingRouletteWheelSelector::CanSelectSameDesignMoreThanOnce(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return false;
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



NonDuplicatingRouletteWheelSelector::NonDuplicatingRouletteWheelSelector(
    GeneticAlgorithm& algorithm
    ) :
        RouletteWheelSelector(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

NonDuplicatingRouletteWheelSelector::NonDuplicatingRouletteWheelSelector(
    const NonDuplicatingRouletteWheelSelector& copy
    ) :
        RouletteWheelSelector(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

NonDuplicatingRouletteWheelSelector::NonDuplicatingRouletteWheelSelector(
    const NonDuplicatingRouletteWheelSelector& copy,
    GeneticAlgorithm& algorithm
    ) :
        RouletteWheelSelector(copy, algorithm)
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
