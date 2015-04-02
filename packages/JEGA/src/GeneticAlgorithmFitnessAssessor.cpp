/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class GeneticAlgorithmFitnessAssessor

    NOTES:

        See notes of GeneticAlgorithmFitnessAssessor.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue May 20 13:11:41 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the GeneticAlgorithmFitnessAssessor
 *        class.
 */





/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmFitnessAssessor.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>



/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA;








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
GeneticAlgorithmFitnessAssessor::PollForParameters(
    const JEGA::Utilities::ParameterDatabase&
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return true;
}

string
GeneticAlgorithmFitnessAssessor::GetType(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return "Fitness Assessor";
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
GeneticAlgorithmFitnessAssessor::GeneticAlgorithmFitnessAssessor(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmOperator(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

GeneticAlgorithmFitnessAssessor::GeneticAlgorithmFitnessAssessor(
    const GeneticAlgorithmFitnessAssessor& copy
    ) :
        GeneticAlgorithmOperator(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

GeneticAlgorithmFitnessAssessor::GeneticAlgorithmFitnessAssessor(
    const GeneticAlgorithmFitnessAssessor& copy,
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
