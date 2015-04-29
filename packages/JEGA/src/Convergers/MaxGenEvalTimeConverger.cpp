/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class MaxGenEvalTimeConverger.

    NOTES:

        See notes of MaxGenEvalTimeConverger.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.7.0

    CHANGES:

        Thu Sep 18 13:43:57 2014 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the MaxGenEvalTimeConverger class.
 */




/*
================================================================================
Includes
================================================================================
*/
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <Convergers/MaxGenEvalTimeConverger.hpp>







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
MaxGenEvalTimeConverger::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("max_evals_gens_time");
    return ret;
}

const string&
MaxGenEvalTimeConverger::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This converger returns true if either the maximum allowable number of "
        "evaluations, maximum allowable number of generations, or maximum wall "
        "clock time has been reached or exceeded."
        );
    return ret;
}

GeneticAlgorithmOperator*
MaxGenEvalTimeConverger::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new MaxGenEvalTimeConverger(algorithm);
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
MaxGenEvalTimeConverger::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return MaxGenEvalTimeConverger::Name();
}

string
MaxGenEvalTimeConverger::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return MaxGenEvalTimeConverger::Description();
}

GeneticAlgorithmOperator*
MaxGenEvalTimeConverger::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new MaxGenEvalTimeConverger(*this, algorithm);
}

bool
MaxGenEvalTimeConverger::CheckConvergence(
    const DesignGroup&,
    const FitnessRecord&
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GeneticAlgorithmConverger::CheckConvergence();
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

MaxGenEvalTimeConverger::MaxGenEvalTimeConverger(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmConverger(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

MaxGenEvalTimeConverger::MaxGenEvalTimeConverger(
    const MaxGenEvalTimeConverger& copy
    ) :
        GeneticAlgorithmConverger(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

MaxGenEvalTimeConverger::MaxGenEvalTimeConverger(
    const MaxGenEvalTimeConverger& copy,
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

