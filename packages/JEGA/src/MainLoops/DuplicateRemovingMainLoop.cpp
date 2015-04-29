/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class DuplicateRemovingMainLoop.

    NOTES:

        See notes of DuplicateRemovingMainLoop.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Jun 12 07:58:21 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the DuplicateRemovingMainLoop class.
 */


/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <FitnessRecord.hpp>
#include <GeneticAlgorithm.hpp>
#include <GeneticAlgorithmCrosser.hpp>
#include <GeneticAlgorithmMutator.hpp>
#include <GeneticAlgorithmNichePressureApplicator.hpp>
#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <MainLoops/DuplicateRemovingMainLoop.hpp>




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
DuplicateRemovingMainLoop::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("duplicate_free");
    return ret;
}

const string&
DuplicateRemovingMainLoop::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This main loop operator behaves exactly as the StandardMainLoop "
        "with two exceptions.  First, prior to fitness assessment, all "
        "offspring Designs that duplicate a population member or another "
        "child Design are removed.  Second, after selection, any non-unique "
        "population members are flushed."
        );
    return ret;
}

GeneticAlgorithmOperator*
DuplicateRemovingMainLoop::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new DuplicateRemovingMainLoop(algorithm);
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
DuplicateRemovingMainLoop::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return DuplicateRemovingMainLoop::Name();
}

string
DuplicateRemovingMainLoop::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return DuplicateRemovingMainLoop::Description();
}

GeneticAlgorithmOperator*
DuplicateRemovingMainLoop::Clone(
  GeneticAlgorithm& algorithm
  ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new DuplicateRemovingMainLoop(*this, algorithm);
}

bool
DuplicateRemovingMainLoop::RunGeneration(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // store a reference to the algorithm for repeated use.
    GeneticAlgorithm& algorithm = this->GetAlgorithm();
    const GeneticAlgorithm& constAlg = algorithm;

    // store a reference to the population and the offspring
    DesignGroup& pop = algorithm.GetPopulation();
    DesignGroup& cldrn = algorithm.GetOffspring();

    // On the first run, the population must be tested.
    // After that it should be safe at this point.
    if(algorithm.GetGenerationNumber() == 0)
    {
        JEGA_LOGGING_IF_ON(DesignDVSortSet::size_type nrem =)
            pop.FlushNonUnique();

        JEGAIFLOG_CF_II(nrem > 0, this->GetLogger(), lverbose(), this,
            ostream_entry(
                lverbose(), this->GetName() + ": encountered and flushed "
                ) << nrem << " non-unique designs from the initial population."
            )
    }

    // Don't bother creating new designs from the initial population.  First
    // apply some selection pressure and then start performing variation.  This
    // is sensible especially if the initial population is randomly generated.
    // if that is the case, then performing the following would result in
    // creating more essentially random designs.  If it is warm started, then
    // the rationale is similar.  If they are good designs, then most of them
    // will survive selection.  If not, then they shouldn't have been explored
    // further anyway.
    else
    {
        // at this point, if there were fitnesses on record, they are no longer
        // valid.
        algorithm.SetCurrentFitnesses(0x0);

        // Do Crossover
        algorithm.DoCrossover();

        // Do Mutation
        algorithm.DoMutation();

        if(constAlg.GetMutator().CanProduceInvalidVariableValues() ||
           constAlg.GetCrosser().CanProduceInvalidVariableValues())
        {
            // at this point, we should verify the variable values of
            // the generated Designs
            const size_t numIll = algorithm.ValidateVariableValues(cldrn);

            // ValidateVariableValues may mark Designs illconditioned if it
            // cannot fix the variable values so we should flush any of those.
            if(numIll > 0)
            {                
                this->GetAlgorithm().LogIllconditionedDesigns(cldrn);

                JEGA_LOGGING_IF_ON(DesignDVSortSet::size_type nrem =)
                    cldrn.FlushIllconditionedDesigns();

                JEGAIFLOG_CF_II(nrem > 0, this->GetLogger(), lquiet(), this,
                    ostream_entry(lquiet(), this->GetName() + ": flushed ")
                        << nrem
                        << " designs whose variables could not be corrected."
                        )
            }
        }

        // detect any cloned children

        // first thing to do is flush clones out of the children list.
        // Clones in the children need not be put in the discards buffer
        // because one of the Designs that they are clones of will wind
        // up there or in the population.
        DesignDVSortSet::size_type nrem = cldrn.FlushNonUnique();

        JEGAIFLOG_CF_II(nrem > 0, this->GetLogger(), lverbose(), this,
            ostream_entry(lverbose(), this->GetName() + ": flushed ") << nrem
                << " duplicate offspring designs after application of the "
                   "variational operators."
                )

        EDDY_ASSERT(cldrn.GetDVSortContainer().count_non_unique() == 0);

        // now check against the population
        nrem = pop.GetDVSortContainer().test_for_clones(
            cldrn.GetDVSortContainer()
            );

        // now flush the clones if there are any.
        if(nrem > 0) nrem = cldrn.FlushCloneDesigns();

        JEGAIFLOG_CF_II(nrem > 0, this->GetLogger(), lverbose(), this,
            ostream_entry(lverbose(), this->GetName() + ": flushed ") << nrem
                << " offspring designs that duplicate existing population "
                   "members."
                )

        EDDY_ASSERT(pop.GetDVSortContainer().test_for_clones(
            cldrn.GetDVSortContainer()
            ) == 0);

        const DesignDVSortSet& discards =
            this->GetDesignTarget().CheckoutDiscards();
        nrem = discards.test_for_clones(cldrn.GetDVSortContainer());
        this->GetDesignTarget().CheckinDiscards();

        if(nrem > 0) nrem = cldrn.FlushCloneDesigns();

        nrem = this->GetAlgorithm().GetOperatorSet().GetNichePressureApplicator(
            ).TestBufferForClones(cldrn.GetDVSortContainer());

        if(nrem > 0) nrem = cldrn.FlushCloneDesigns();

        JEGAIFLOG_CF_II(nrem > 0, this->GetLogger(), lverbose(), this,
            ostream_entry(lverbose(), this->GetName() + ": flushed ") << nrem
                << " offspring designs that duplicate discarded designs from "
                   "previous generations."
                )

        // now evaluate the children
        const bool evald = algorithm.DoEvaluation(cldrn);

        JEGAIFLOG_CF_II(!evald, this->GetLogger(), lquiet(), this,
            text_entry(lquiet(), this->GetName() + ": Errors were encountered "
                "while evaluating the offspring designs")
            )

        // any children that could not be evaluated (are illconditioned) must
        // be removed.  Issue a log entry about any removed.
        if(!evald)
        {
            this->GetAlgorithm().LogIllconditionedDesigns(cldrn);

            JEGA_LOGGING_IF_ON(nrem =) cldrn.FlushIllconditionedDesigns();

            JEGAIFLOG_CF_II(nrem > 0, this->GetLogger(), lquiet(), this,
                ostream_entry(
                lquiet(), this->GetName() + ": encountered and flushed "
                )
                    << nrem
                    << " illconditioned designs after evaluation of children."
                    )
        }
    }

    // now merge in any designs injected via the evaluator.  They get merged
    // into the children and must already be evaluated.
    algorithm.AbsorbEvaluatorInjections(false);

    // now perform the pre-selection operation whereby any of the
    // selection operators that need it get an opportunity to prepare.
    algorithm.DoPreSelection();

    // assess the fitness of the current designs.
    const FitnessRecord* fits(algorithm.DoFitnessAssessment());
    algorithm.SetCurrentFitnesses(fits);

    // do not alter the groups after fitness assessment.  This could cause the
    // fitness statistics to become invalidated and selection to fail.

    // Do selection of next population.
    algorithm.DoSelection(*fits);

    // flush non-unique designs.
    JEGA_LOGGING_IF_ON(DesignDVSortSet::size_type nrem =)
        pop.FlushNonUnique();

    JEGAIFLOG_CF_II(nrem > 0, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": flushed ") << nrem
            << " duplicates found in the population after selection.  "
               "This must be caused by use of a selector that does not "
               "respect the duplicate free requirement or a niche pressure "
               "applicator whose pre-selection operation doesn't."
            )

    // in debug mode, assert that the duplicate removal stuff worked
    EDDY_ASSERT(pop.GetDVSortContainer().count_non_unique() == 0);

    // Do secondary selection by niche pressure of next population.
    algorithm.ApplyNichePressure(pop, *fits);

    // increment the generation number.
    this->IncCurrentGeneration();

    // if converged, set to the complete state.
    const bool ret = !algorithm.TestForConvergence(*fits);

    this->PostLoopReport();

    // return whether or not we converged.
    return ret;
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

DuplicateRemovingMainLoop::DuplicateRemovingMainLoop(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmMainLoop(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

DuplicateRemovingMainLoop::DuplicateRemovingMainLoop(
    const DuplicateRemovingMainLoop& copy
    ) :
        GeneticAlgorithmMainLoop(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

DuplicateRemovingMainLoop::DuplicateRemovingMainLoop(
    const DuplicateRemovingMainLoop& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmMainLoop(copy, algorithm)
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
