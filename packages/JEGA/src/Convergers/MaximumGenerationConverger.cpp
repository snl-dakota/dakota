/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class MaximumGenerationConverger.

    NOTES:

        See notes of MaximumGenerationConverger.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Jun 11 07:13:23 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the MaximumGenerationConverger class.
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
#include <Convergers/MaximumGenerationConverger.hpp>





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
MaximumGenerationConverger::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("max_generations");
    return ret;
}

const string&
MaximumGenerationConverger::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This converger returns true if the maximum allowable "
        "number of generations has been reached or exceeded."
        );

    return ret;
}

GeneticAlgorithmOperator*
MaximumGenerationConverger::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new MaximumGenerationConverger(algorithm);
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
MaximumGenerationConverger::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return MaximumGenerationConverger::Name();
}

string
MaximumGenerationConverger::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return MaximumGenerationConverger::Description();
}

GeneticAlgorithmOperator*
MaximumGenerationConverger::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new MaximumGenerationConverger(*this, algorithm);
}

bool
MaximumGenerationConverger::CheckConvergence(
    const DesignGroup&,
    const FitnessRecord&
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->CheckConvergence();
}

bool
MaximumGenerationConverger::CheckConvergence(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(ldebug(), this->GetName() + ": Entering convergence check.")
        )

    // convergence depends only on the maximum allowable
    // number of generations.
    this->SetConverged(this->IsMaxGensReached());

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

MaximumGenerationConverger::MaximumGenerationConverger(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmConverger(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

MaximumGenerationConverger::MaximumGenerationConverger(
    const MaximumGenerationConverger& copy
    ) :
        GeneticAlgorithmConverger(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

MaximumGenerationConverger::MaximumGenerationConverger(
    const MaximumGenerationConverger& copy,
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
