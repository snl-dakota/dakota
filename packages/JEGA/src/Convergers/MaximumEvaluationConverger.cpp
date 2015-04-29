/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class MaximumEvaluationConverger.

    NOTES:

        See notes of MaximumEvaluationConverger.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Jun 11 07:13:12 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the MaximumEvaluationConverger class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <Convergers/MaximumEvaluationConverger.hpp>







/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Utilities;
using namespace JEGA::Logging;








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
MaximumEvaluationConverger::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("max_evaluations");
    return ret;
}

const string&
MaximumEvaluationConverger::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This converger returns true if the maximum allowable "
        "number of evaluations has been reached or exceeded."
        );
    return ret;
}

GeneticAlgorithmOperator*
MaximumEvaluationConverger::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new MaximumEvaluationConverger(algorithm);
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
MaximumEvaluationConverger::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return MaximumEvaluationConverger::Name();
}

string
MaximumEvaluationConverger::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return MaximumEvaluationConverger::Description();
}

GeneticAlgorithmOperator*
MaximumEvaluationConverger::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new MaximumEvaluationConverger(*this, algorithm);
}

bool
MaximumEvaluationConverger::CheckConvergence(
    const DesignGroup&,
    const FitnessRecord&
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->CheckConvergence();
}

bool
MaximumEvaluationConverger::CheckConvergence(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(ldebug(), this->GetName() + ": Entering convergence check.")
        )

    // convergence depends only on the maximum allowable
    // number of evaluations.
    this->SetConverged(this->IsMaxEvalsReached());

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

MaximumEvaluationConverger::MaximumEvaluationConverger(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmConverger(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

MaximumEvaluationConverger::MaximumEvaluationConverger(
    const MaximumEvaluationConverger& copy
    ) :
        GeneticAlgorithmConverger(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

MaximumEvaluationConverger::MaximumEvaluationConverger(
    const MaximumEvaluationConverger& copy,
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
