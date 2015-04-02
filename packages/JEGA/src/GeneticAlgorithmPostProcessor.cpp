/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class GeneticAlgorithmPostProcessor

    NOTES:

        See notes of GeneticAlgorithmPostProcessor.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Thu Aug 17 15:26:36 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the GeneticAlgorithmPostProcessor
 *        class.
 */





/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <FitnessRecord.hpp>
#include <../Utilities/include/Logging.hpp>
#include <GeneticAlgorithmPostProcessor.hpp>
#include <../Utilities/include/DesignGroup.hpp>
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
GeneticAlgorithmPostProcessor::PollForParameters(
    const JEGA::Utilities::ParameterDatabase&
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return true;
}

string
GeneticAlgorithmPostProcessor::GetType(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return "Post Processor";
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
GeneticAlgorithmPostProcessor::GeneticAlgorithmPostProcessor(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmOperator(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

GeneticAlgorithmPostProcessor::GeneticAlgorithmPostProcessor(
    const GeneticAlgorithmPostProcessor& copy
    ) :
        GeneticAlgorithmOperator(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

GeneticAlgorithmPostProcessor::GeneticAlgorithmPostProcessor(
    const GeneticAlgorithmPostProcessor& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmOperator(copy, algorithm)
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
