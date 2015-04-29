/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class MaxGenEvalConverger.

    NOTES:

        See notes of MaxGenEvalConverger.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Jun 11 07:13:38 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the MaxGenEvalConverger class.
 */


/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/Logging.hpp>
#include <Convergers/MaxGenEvalConverger.hpp>
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
MaxGenEvalConverger::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("max_evals_or_gens");
    return ret;
}

const string&
MaxGenEvalConverger::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This converger returns true if either the maximum allowable "
        "number of evaluations or maximum allowable number of "
        "generations has been reached or exceeded."
        );
    return ret;
}

GeneticAlgorithmOperator*
MaxGenEvalConverger::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new MaxGenEvalConverger(algorithm);
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
MaxGenEvalConverger::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return MaxGenEvalConverger::Name();
}

string
MaxGenEvalConverger::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return MaxGenEvalConverger::Description();
}

GeneticAlgorithmOperator*
MaxGenEvalConverger::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new MaxGenEvalConverger(*this, algorithm);
}

bool
MaxGenEvalConverger::CheckConvergence(
    const DesignGroup&,
    const FitnessRecord&
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->CheckConvergence();
}

bool
MaxGenEvalConverger::CheckConvergence(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(ldebug(), this->GetName() + ": Entering convergence check.")
        )

    // convergence depends only on the maximum allowable number
    // of generations and maximum allowable number of evaluations.
    // If either has been reached or exceeded, we've converged.
    bool converged = this->IsMaxGensReached() || this->IsMaxEvalsReached();

    JEGAIFLOG_CF_II(this->IsMaxGensReached(), this->GetLogger(), lverbose(),
        this,
        text_entry(lverbose(), this->GetName() + ": maximum allowable number "
            "of generations has been reached.")
        )

    JEGAIFLOG_CF_II(this->IsMaxEvalsReached(), this->GetLogger(), lverbose(),
        this,
        text_entry(lverbose(), this->GetName() + ": maximum allowable number "
            "of evaluations has been reached.")
        )

    this->SetConverged(converged);

    // return true if we have converged, false otherwise.
    return this->GetConverged();
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

MaxGenEvalConverger::MaxGenEvalConverger(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmConverger(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

MaxGenEvalConverger::MaxGenEvalConverger(
    const MaxGenEvalConverger& copy
    ) :
        GeneticAlgorithmConverger(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

MaxGenEvalConverger::MaxGenEvalConverger(
    const MaxGenEvalConverger& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmConverger(copy, algorithm)
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
