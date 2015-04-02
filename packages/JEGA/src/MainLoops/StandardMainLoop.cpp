/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class StandardMainLoop.

    NOTES:

        See notes of StandardMainLoop.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Jun 04 11:05:38 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the StandardMainLoop class.
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
#include <MainLoops/StandardMainLoop.hpp>
#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>




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
StandardMainLoop::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("standard");
    return ret;
}

const string&
StandardMainLoop::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This main loop executes the typical main loop of a GA.  "
        "The algorithm proceeds as shown (recall that initialization "
        "has already occurred):\n\n"
        "    while not Converged\n"
        "         Crossover\n"
        "         Mutate\n"
        "         Evaluate\n"
        "         Assess Fitness\n"
        "         Niche Pressure Application\n"
        "         Select\n"
        "    end while"
        );
    return ret;
}

GeneticAlgorithmOperator*
StandardMainLoop::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new StandardMainLoop(algorithm);
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
StandardMainLoop::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return StandardMainLoop::Name();
}

string
StandardMainLoop::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return StandardMainLoop::Description();
}

GeneticAlgorithmOperator*
StandardMainLoop::Clone(
  GeneticAlgorithm& algorithm
  ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new StandardMainLoop(*this, algorithm);
}

bool
StandardMainLoop::RunGeneration(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // store a reference to the algorithm for repeated use.
    GeneticAlgorithm& algorithm = this->GetAlgorithm();
    const GeneticAlgorithm& constAlg = algorithm;

    DesignGroup& pop = algorithm.GetPopulation();
    DesignGroup& cldrn = algorithm.GetOffspring();

    // Don't bother creating new designs from the initial population.  First
    // apply some selection pressure and then start performing variation.  This
    // is sensible especially if the initial population is randomly generated.
    // if that is the case, then performing the following would result in
    // creating more essentially random designs.  If it is warm started, then
    // the rationale is similar.  If they are good designs, then most of them
    // will survive selection.  If not, then they shouldn't have been explored
    // further anyway.
    if(algorithm.GetGenerationNumber() != 0)
    {
        // at this point, if there were fitnesses on record, they are no longer
        // valid.
        algorithm.SetCurrentFitnesses(0x0);

        // Do Crossover
        algorithm.DoCrossover();

        // put the
        // Do Mutation
        algorithm.DoMutation();

        if(constAlg.GetMutator().CanProduceInvalidVariableValues() ||
           constAlg.GetCrosser().CanProduceInvalidVariableValues())
        {
            // at this point, we should verify the variable values of
            // the generated Designs
            size_t numIll = algorithm.ValidateVariableValues(cldrn);

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

        // Check for clones to avoid duplicate evaluations both within the
        // children and amongst the population
        pop.GetDVSortContainer().test_for_clones(cldrn.GetDVSortContainer());
        cldrn.GetDVSortContainer().test_within_list_for_clones();

        const DesignDVSortSet& discards =
            this->GetDesignTarget().CheckoutDiscards();
        discards.test_for_clones(cldrn.GetDVSortContainer());
        this->GetDesignTarget().CheckinDiscards();

        // now evaluate the children.  This will resolve any clones responses
        // prior to invoking the evaluator.  The evaluator may then choose not
        // to re-evaluate.
        bool evald = algorithm.DoEvaluation(cldrn);

        JEGAIFLOG_CF_II(!evald, this->GetLogger(), lquiet(), this,
            text_entry(lquiet(), this->GetName() + ": Errors were "
                "encountered while evaluating the offspring designs")
            )

        // any children that could not be evaluated (are illconditioned) must
        // be removed.  Issue a log entry about any removed.
        if(!evald)
        {
            this->GetAlgorithm().LogIllconditionedDesigns(cldrn);

            JEGA_LOGGING_IF_ON(DesignDVSortSet::size_type nrem =)
                cldrn.FlushIllconditionedDesigns();

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
    algorithm.AbsorbEvaluatorInjections(true);

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

    // Do secondary selection by niche pressure of next population.
    algorithm.ApplyNichePressure(pop, *fits);

    // increment the generation number.
    this->IncCurrentGeneration();

    // if converged, set to the complete state.
    bool ret = !algorithm.TestForConvergence(*fits);

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
StandardMainLoop::StandardMainLoop(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmMainLoop(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

StandardMainLoop::StandardMainLoop(
    const StandardMainLoop& copy
    ) :
        GeneticAlgorithmMainLoop(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

StandardMainLoop::StandardMainLoop(
    const StandardMainLoop& copy,
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
