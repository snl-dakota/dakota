/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class GeneticAlgorithmSelector

    NOTES:

        See notes of GeneticAlgorithmSelector.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed May 21 15:26:36 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the GeneticAlgorithmSelector class.
 */





/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <cfloat>
#include <FitnessRecord.hpp>
#include <GeneticAlgorithmSelector.hpp>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignGroupVector.hpp>






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

void
GeneticAlgorithmSelector::RemoveFromGroups(
    const Design* design,
    const DesignGroupVector& groups
    )
{
    EDDY_FUNC_DEBUGSCOPE
    for(DesignGroupVector::const_iterator it(groups.begin());
        it!=groups.end(); ++it) (*it)->Erase(design);
}


/*
================================================================================
Subclass Overridable Methods
================================================================================
*/
bool
GeneticAlgorithmSelector::PollForParameters(
    const JEGA::Utilities::ParameterDatabase&
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return true;
}

string
GeneticAlgorithmSelector::GetType(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return "Selector";
}

DesignOFSortSet
GeneticAlgorithmSelector::SelectNBest(
    DesignGroupVector& from,
    std::size_t n,
    const FitnessRecord& ftns
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        ostream_entry(ldebug(), this->GetName() + ": identifying the ")
            << n << " best designs."
        )

    return this->SelectNBest<FitnessPred>(from, n, FitnessPred(ftns));
}

bool
GeneticAlgorithmSelector::CanSelectSameDesignMoreThanOnce(
    ) const
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
GeneticAlgorithmSelector::GeneticAlgorithmSelector(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmOperator(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

GeneticAlgorithmSelector::GeneticAlgorithmSelector(
    const GeneticAlgorithmSelector& copy
    ) :
        GeneticAlgorithmOperator(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

GeneticAlgorithmSelector::GeneticAlgorithmSelector(
    const GeneticAlgorithmSelector& copy,
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
