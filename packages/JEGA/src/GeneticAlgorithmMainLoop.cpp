/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class GeneticAlgorithmMainLoop

    NOTES:

        See notes of GeneticAlgorithmMainLoop.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed May 21 15:26:50 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the GeneticAlgorithmMainLoop class.
 */


/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithm.hpp>
#include <GeneticAlgorithmMainLoop.hpp>
#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>






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
GeneticAlgorithmMainLoop::PollForParameters(
    const JEGA::Utilities::ParameterDatabase&
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return true;
}

string
GeneticAlgorithmMainLoop::GetType(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return "Main Loop";
}

void
GeneticAlgorithmMainLoop::PostLoopReport(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA_LOGGING_IF_ON(
        eddy::utilities::uint64_t nGen =
            this->GetAlgorithm().GetGenerationNumber();
        eddy::utilities::uint64_t nEval =
            this->GetAlgorithm().GetNumberEvaluations();
        )

    JEGALOG_II(this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(), this->GetName() + ": ")
        << nGen << " generation" << (nGen==1 ? " " : "s ") << "so far."
        )

    JEGALOG_II(this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(), this->GetName() + ": ")
            << nEval << " evaluation" << (nEval==1 ? " " : "s ") << "so far."
        )

    // Do this regardless of logging b/c the last entry is a fatal entry which
    // happens even when not logging.
    DesignDVSortSet::size_type popSize =
        this->GetAlgorithm().GetPopulation().GetSize();

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": ")
            << popSize << " member" << (popSize==1 ? " " : "s ")
            << "in population so far."
        )

    JEGAIFLOG_CF_II_F(popSize < 2, this->GetLogger(), this,
        text_entry(lfatal(), this->GetName() + ": The population size has "
            "fallen below the minimum allowable value of 2.")
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
GeneticAlgorithmMainLoop::GeneticAlgorithmMainLoop(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmOperator(algorithm),
        _currGen(0)
{
    EDDY_FUNC_DEBUGSCOPE
}

GeneticAlgorithmMainLoop::GeneticAlgorithmMainLoop(
    const GeneticAlgorithmMainLoop& copy
    ) :
        GeneticAlgorithmOperator(copy),
        _currGen(0)
{
    EDDY_FUNC_DEBUGSCOPE
}

GeneticAlgorithmMainLoop::GeneticAlgorithmMainLoop(
    const GeneticAlgorithmMainLoop& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmOperator(copy, algorithm),
        _currGen(0)
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
