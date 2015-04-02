/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class MOGA.

    NOTES:

        See notes of MOGA.hpp.

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
 * \brief Contains the implementation of the MOGA class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <cfloat>
#include <memory> // for auto_ptr
#include <../MOGA/include/MOGA.hpp>
#include <utilities/include/extremes.hpp>
#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignStatistician.hpp>
#include <../Utilities/include/MultiObjectiveStatistician.hpp>
#include <../MOGA/include/OperatorGroups/MOGAOperatorGroup.hpp>
#include <../MOGA/include/OperatorGroups/DominationCountOperatorGroup.hpp>

/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
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
const bool MOGA::_registered_operator_groups =


    MOGA::RegistryOfOperatorGroups().register_(
        MOGAOperatorGroup::Name(),
        &MOGAOperatorGroup::Instance) &&


    MOGA::RegistryOfOperatorGroups().register_(
        DominationCountOperatorGroup::Name(),
        &DominationCountOperatorGroup::Instance) &&


true;





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
GeneticAlgorithmOperatorGroupRegistry&
MOGA::RegistryOfOperatorGroups(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static GeneticAlgorithmOperatorGroupRegistry registry;
    return registry;
}

void
MOGA::ReclaimOptimal(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(
            ldebug(),
            this->GetName() +
            ": Reclaiming optimal designs from target discards."
            )
        )

    // Go through all the discarded Designs and re-claim any
    // that are non-dominated but got selected out.
    DesignTarget& target = this->GetDesignTarget();

    // store the discards for repeated use.
    const DesignDVSortSet& discards = target.CheckoutDiscards();

    // Create a DesignGroup from the discards so that we'll have something we
    // can iterate while reclaiming designs and so we'll have an OF sort set.
    DesignGroup discardGroup(target, discards);
    discardGroup.FlushNonEvaluatedDesigns();

    // Get the current population.
    DesignGroup& pop = this->GetPopulation();
    pop.FlushNonEvaluatedDesigns();

    // Also get a reference to pop's DesignOFSortSet
    const DesignOFSortSet& popDeque = pop.GetOFSortContainer();

    // iterate over the Designs in the discards and take any
    // that we find to be non-dominated in the population.
    JEGA_LOGGING_IF_ON(size_t numReclaimed = 0;)
    for(DesignOFSortSet::iterator it(discardGroup.BeginOF());
        it!=discardGroup.EndOF(); ++it)
    {
        if(!MultiObjectiveStatistician::IsDominatedByAtLeast1(**it, popDeque))
        {
            // Get the discarded Design out of the discards.
            if(target.ReclaimDesign(**it))
            {
                // now put the discarded Design into our population.
                pop.Insert(*it);
                JEGA_LOGGING_IF_ON(++numReclaimed;)
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

const Design*
MOGA::GetBestDesign(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Retrieve any potentially lost Optimal designs
    this->ReclaimOptimal();

    // Store the population for repeated use.
    DesignGroup& pop = this->GetPopulation();

    if(pop.CountFeasible() != 0)
    {
        // There are feasible designs so the infeasible are of
        // no interest.  So flush them.
        this->GetPopulation().FlushNonFeasibleDesigns();

        //now find and store those extremes
        DoubleExtremes extremeSet(
            MultiObjectiveStatistician::FindParetoExtremes(
                pop.GetOFSortContainer()
                )
            );

        //get number of objective functions
        const std::size_t nof = this->GetDesignTarget().GetNOF();
        EDDY_ASSERT(nof == extremeSet.size());

        // This will store the minimum squared deviation from the
        // utopia point.  Initialize it to a very large number.
        double minDistance = DBL_MAX;

        // Create a pointer to store the best design as each is
        // considered.  Initialize it to the null pointer.
        const Design* bestDesign = 0x0;

        // prepare to iterate the designs by objective function
        // and find the best.
        DesignOFSortSet::const_iterator it(pop.BeginOF());
        const DesignOFSortSet::const_iterator e(pop.EndOF());

        for(; it!=e; ++it)
        {
            // prepare a place to store sum-of-squared distance
            // for this Design as we traverse the objective functions.
            double tempDistance = 0.0;

            // compute the sum-of-squares between the current point and
            // the utopia point.  Store it as tempDistance.
            for(size_t i=0; i<nof; ++i)
            {
                const double objDist =
                    (*it)->GetObjective(i)-extremeSet.get_min(i);
                tempDistance += (objDist * objDist);
            }

            // now, if this Design turns out to be closer to the utopia
            // point than any previously considered Design, take it as
            // the best and update our current minimum distance.
            if (tempDistance < minDistance)
            {
                bestDesign = *it;
                minDistance = tempDistance;
            }
        }
        // now return our best Design.  The only way this value
        // could be null at this point is if every Design in our feasible
        // set has sum-of-squares distance from the utopia point == DBL_MAX.
        return bestDesign;
    }

    else

        // If there are no feasible designs, we are here and we return
        // a null pointer indicating that there is no best.  Perhaps in
        // the future, we should search for the "best of the worst".
        return 0x0;

}


GeneticAlgorithmOperatorGroupRegistry&
MOGA::GetOperatorGroupRegistry(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return RegistryOfOperatorGroups();
}

void
MOGA::FlushNonOptimal(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA_LOGGING_IF_ON(const std::size_t numRem = )
        MultiObjectiveStatistician::FlushDominatedFrom(this->GetPopulation());

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": Flushed ")
            << numRem << " dominated designs from the population."
        )
}

DesignOFSortSet
MOGA::GetCurrentSolution(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    const DesignGroup& pop = this->GetPopulation();
    const DesignTarget& target = this->GetDesignTarget();

    // if we've finalized, the current solution is in the population.
    // There is no need to search the discards, etc.
    if(this->IsFinalized()) return pop.GetOFSortContainer();

    // Get the pareto from the population.
    DesignOFSortSet pareto(MultiObjectiveStatistician::GetNonDominated(
        pop.GetOFSortContainer()
        ));

    const DesignDVSortSet& discards = target.CheckoutDiscards();

    // popPareto is either all feasible or all infeasible.  Whatever the
    // case, we will add any non-dominated designs from the discards back in.
    for(DesignDVSortSet::const_iterator it(discards.begin());
        it!=discards.end(); ++it)
    {
        if(!MultiObjectiveStatistician::IsDominatedByAtLeast1(**it, pareto))
            pareto.insert(*it);
    }

    // we are done with the discards now and can check them back in.
    target.CheckinDiscards();

    return pareto;
}

string
MOGA::GetAlgorithmTypeName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return "moga";
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
MOGA::MOGA(
    DesignTarget& target,
    Logger& logger
    ) :
        GeneticAlgorithm(target, logger)
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
