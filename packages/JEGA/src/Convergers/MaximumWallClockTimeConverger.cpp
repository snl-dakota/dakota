/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class MaximumWallClockTimeConverger.

    NOTES:

        See notes of MaximumWallClockTimeConverger.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.7.0

    CHANGES:

        Thu Sep 11 08:20:51 2014 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the MaximumWallClockTimeConverger class.
 */




/*
================================================================================
Includes
================================================================================
*/
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <Convergers/MaximumWallClockTimeConverger.hpp>







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
MaximumWallClockTimeConverger::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("max_time");
    return ret;
}

const string&
MaximumWallClockTimeConverger::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This converger returns true if the maximum allowable "
        "amount of time has been reached or exceeded."
        );
    return ret;
}

GeneticAlgorithmOperator*
MaximumWallClockTimeConverger::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new MaximumWallClockTimeConverger(algorithm);
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
MaximumWallClockTimeConverger::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return MaximumWallClockTimeConverger::Name();
}

string
MaximumWallClockTimeConverger::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return MaximumWallClockTimeConverger::Description();
}

GeneticAlgorithmOperator*
MaximumWallClockTimeConverger::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new MaximumWallClockTimeConverger(*this, algorithm);
}

bool
MaximumWallClockTimeConverger::CheckConvergence(
    const DesignGroup&,
    const FitnessRecord&
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->CheckConvergence();
}

bool
MaximumWallClockTimeConverger::CheckConvergence(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(ldebug(), this->GetName() + ": Entering convergence check.")
        )

    // convergence depends only on the maximum allowable
    // number of evaluations.
    this->SetConverged(this->IsMaxTimeReached());

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

MaximumWallClockTimeConverger::MaximumWallClockTimeConverger(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmConverger(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

MaximumWallClockTimeConverger::MaximumWallClockTimeConverger(
    const MaximumWallClockTimeConverger& copy
    ) :
        GeneticAlgorithmConverger(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

MaximumWallClockTimeConverger::MaximumWallClockTimeConverger(
    const MaximumWallClockTimeConverger& copy,
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

