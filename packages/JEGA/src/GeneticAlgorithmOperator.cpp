/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class GeneticAlgorithmOperator

    NOTES:

        See notes of GeneticAlgorithmOperator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue May 13 13:04:06 2003 - Original Version (JE)
        Wed Dec 07 15:30:05 2005 - Made modifications necessary for replacement
                                   of ProblemDescDB with ParameterDatabase.(JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the GeneticAlgorithmOperator class.
 */





/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithm.hpp>
#include <GeneticAlgorithmOperator.hpp>
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


bool
GeneticAlgorithmOperator::ExtractParameters(
    const ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->PollForParameters(db);
}

const DesignTarget&
GeneticAlgorithmOperator::GetDesignTarget(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetAlgorithm().GetDesignTarget();
}




/*
================================================================================
Subclass Visible Methods
================================================================================
*/

DesignTarget&
GeneticAlgorithmOperator::GetDesignTarget(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetAlgorithm().GetDesignTarget();
}

Logger&
GeneticAlgorithmOperator::GetLogger(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetAlgorithm().GetLogger();
}

const string&
GeneticAlgorithmOperator::GetAlgorithmName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetAlgorithm().GetName();
}

/*
================================================================================
Subclass Overridable Methods
================================================================================
*/

bool
GeneticAlgorithmOperator::Finalize(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return true;
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
GeneticAlgorithmOperator::GeneticAlgorithmOperator(
    GeneticAlgorithm& algorithm
    ) :
        _algorithm(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

GeneticAlgorithmOperator::GeneticAlgorithmOperator(
    const GeneticAlgorithmOperator& copy
    ) :
        _algorithm(copy._algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

GeneticAlgorithmOperator::GeneticAlgorithmOperator(
    const GeneticAlgorithmOperator&,
    GeneticAlgorithm& algorithm
    ) :
        _algorithm(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

GeneticAlgorithmOperator::~GeneticAlgorithmOperator(
    )
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
