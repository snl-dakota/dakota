/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class BelowLimitSelector.

    NOTES:

        See notes of BelowLimitSelector.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Jun 11 07:12:50 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the BelowLimitSelector class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <limits>
#include <FitnessRecord.hpp>
#include <utilities/include/Math.hpp>
#include <../Utilities/include/Logging.hpp>
#include <Selectors/BelowLimitSelector.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignGroupVector.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>


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
const double BelowLimitSelector::DEFAULT_SHRINK_PCT(0.9);
const double BelowLimitSelector::DEFAULT_LIMIT(5.0);
const std::size_t BelowLimitSelector::DEFAULT_MIN_SELS(2);








/*
================================================================================
Mutators
================================================================================
*/


void
BelowLimitSelector::SetLimit(
    double limit
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_limit = limit;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": Limiting value now = ") << this->_limit
        )
}


void
BelowLimitSelector::SetShrinkagePercentage(
    double shrinkage
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(shrinkage >= 0.0);

    JEGAIFLOG_CF_II(shrinkage > 1.0, this->GetLogger(), lquiet(), this,
        text_entry(lquiet(), this->GetName() + ": Enforcing >= 100% selection "
                   "at each generation.")
        )

    JEGAIFLOG_CF_II(shrinkage < 0.0, this->GetLogger(), lquiet(), this,
        text_entry(lquiet(), this->GetName() + ": Adjusting shrinking "
                   "parameter to minimum allowable value of 0.0.")
        )

    this->_shrinkage = Math::Max<double>(shrinkage, 0.0);

    JEGALOG_II(GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": Shrinkage percentage now = "
            ) << this->_shrinkage
        )

}

void
BelowLimitSelector::SetMinimumSelections(
    size_t minSels
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // This is not limited to 2 b/c this selector may only be doing part of
    // the selection job.  If at least 2 are not selected in total, JEGA will
    // bomb out.
    this->_minSels = minSels;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": Minimum selections now = ") << this->_minSels
        )

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


const string&
BelowLimitSelector::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("below_limit");
    return ret;
}

const string&
BelowLimitSelector::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This selection operator chooses only those designs that "
        "have a fitness value below a certain limit ("
        "subject to a minimum number of selections)."
        );
    return ret;
}

GeneticAlgorithmOperator*
BelowLimitSelector::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new BelowLimitSelector(algorithm);
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
BelowLimitSelector::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return BelowLimitSelector::Name();
}

string
BelowLimitSelector::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return BelowLimitSelector::Description();
}

GeneticAlgorithmOperator*
BelowLimitSelector::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new BelowLimitSelector(*this, algorithm);
}

bool
BelowLimitSelector::PollForParameters(
    const JEGA::Utilities::ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    bool success = ParameterExtractor::GetDoubleFromDB(
        db, "method.jega.fitness_limit", this->_limit
        );

    // If we did not find the fitness limit, warn about it and use the default
    // value.  Note that if !success, then _limit has not been altered.
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The fitness limit was "
            "not found in the parameter database.  Using the current value of ")
            << this->_limit
        )

    this->SetLimit(this->_limit);

    success = ParameterExtractor::GetDoubleFromDB(
        db, "method.jega.shrinkage_percentage", this->_shrinkage
        );

    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The shrinkage "
            "percentage was not found in the parameter database.  Using the "
            "current value of ") << this->_shrinkage
        )

    this->SetShrinkagePercentage(this->_shrinkage);

    success = ParameterExtractor::GetSizeTypeFromDB(
        db, "method.jega.minimum_selections", this->_minSels
        );

    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The minimum "
            "selections was not found in the parameter database.  Using the "
            "current value of ") << this->_minSels
        )

    this->SetMinimumSelections(this->_minSels);

    return this->GeneticAlgorithmSelector::PollForParameters(db);
}

void
BelowLimitSelector::Select(
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

    // look for the abort conditions.
    if(from.empty()) return;

    JEGA_LOGGING_IF_ON(const std::size_t prevSize = into.GetSize();)

    // Going to need these iterators
    DesignGroupVector::const_iterator it(from.begin());
    const DesignGroupVector::const_iterator e(from.end());

    EDDY_DEBUGEXEC(const DesignTarget& target = this->GetDesignTarget())

    // prepare the groups for this operator.
    size_t numDes = from.GetTotalDesignCount();

    // get the relavant configuration parameters for repeated use.
    const double selCutoff = this->GetLimit();
    const double shrink = this->GetShrinkagePercentage();

    // figure out the minimum and maximum number of selections.
    const size_t maxSels = std::numeric_limits<size_t>::max();

    const size_t minSels = Math::Max(Math::Min<size_t>(
        static_cast<size_t>(Math::Round(shrink * count)), maxSels
        ), this->_minSels);

    // There is a potential shortcut here but I can't imagine when it
    // might actually come to use.  If the minimum number of selections
    // is >= the total number of designs to choose from, we will just
    // take them all and return.
    if(minSels >= numDes)
    {
        for(it=from.begin(); it!=e; ++it)
        {
            DesignOFSortSet::const_iterator dit((*it)->BeginOF());
            for(; dit!=(*it)->EndOF(); ++dit) into.Insert(*dit);
            (*it)->ClearContainers();
        }
        return;
    }

    // Anyway, back to reality.

    // Here is what we are going to do.  This is an elitest algorithm
    // in that it take the best first and stops when it is full.
    // It is full when any of the following have occurred:
    //
    // 1 - The maximum allowable number of selections have taken place.
    // 2 - All designs with fitness below "selCutoff" have been selected
    //     And the minimum number of designs have been selected.

    // iterate all the groups and make selections.
    for(it=from.begin(); it!=from.end(); ++it)
    {
        // store our current group for readability.
        DesignGroup& group = **it;

        for(DesignOFSortSet::iterator dit(group.BeginOF()); dit!=group.EndOF();)
        {
            // Get this designs fitness.  The value should be the negative
            // of what we are actually interested in.  So negate it.
            double currFit = -fitnesses.GetFitness(**dit);

            // if this count is below the limit, then keep it.
            if(currFit <= selCutoff)
            {
                into.Insert(*dit);
                dit = group.EraseRetOF(dit);

                // This checks to see if we have made enough selections
                // yet. Start with bullet 1 from above.
                if(into.GetSize() >= maxSels)
                {
                    JEGALOG_II(this->GetLogger(), lverbose(), this,
                        ostream_entry(lverbose(), this->GetName() +
                            ": Performed ")
                            << (into.GetSize() - prevSize)
                            << " total selections."
                        )
                    return;
                }
            }
            else ++dit;
        }
    }

    // Otherwise, if we haven't made enough selections yet, we have to
    // take some more.  We will take the best of what remains up to the
    // number that we need to fill the group.
    if(into.GetSize() < minSels) this->SelectNBest(
        from, into, minSels - into.GetSize(), fitnesses, FitnessPred(fitnesses)
        );

    // Create a message about our actions here.
    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": Performed ")
            << (into.GetSize() - prevSize) << " total selections."
        )
}

bool
BelowLimitSelector::CanSelectSameDesignMoreThanOnce(
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

bool
BelowLimitSelector::AreAllGroupsEmtpy(
    const DesignGroupVector& groups
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    DesignGroupVector::const_iterator it(groups.begin());
    for(; it!=groups.end(); ++it) if(!(*it)->IsEmpty()) return false;
    return true;
}


/*
================================================================================
Structors
================================================================================
*/


BelowLimitSelector::BelowLimitSelector(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmSelector(algorithm),
        _shrinkage(DEFAULT_SHRINK_PCT),
        _limit(DEFAULT_LIMIT),
        _minSels(DEFAULT_MIN_SELS)
{
    EDDY_FUNC_DEBUGSCOPE
}

BelowLimitSelector::BelowLimitSelector(
    const BelowLimitSelector& copy
    ) :
        GeneticAlgorithmSelector(copy),
        _shrinkage(copy._shrinkage),
        _limit(copy._limit),
        _minSels(copy._minSels)
{
    EDDY_FUNC_DEBUGSCOPE
}

BelowLimitSelector::BelowLimitSelector(
    const BelowLimitSelector& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmSelector(copy, algorithm),
        _shrinkage(copy._shrinkage),
        _limit(copy._limit),
        _minSels(copy._minSels)
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
