/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class ElitistSelector.

    NOTES:

        See notes of ElitistSelector.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Sep 18 16:10:28 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the ElitistSelector class.
 */





/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <Selectors/ElitistSelector.hpp>
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

const string&
ElitistSelector::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("elitist");
    return ret;
}

const string&
ElitistSelector::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret(
        ""
        );
    return ret;
}

GeneticAlgorithmOperator*
ElitistSelector::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new ElitistSelector(algorithm);
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
ElitistSelector::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return ElitistSelector::Name();
}

string
ElitistSelector::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return ElitistSelector::Description();
}

GeneticAlgorithmOperator*
ElitistSelector::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new ElitistSelector(*this, algorithm);
}

void
ElitistSelector::Select(
    DesignGroupVector& from,
    DesignGroup& into,
    size_t count,
    const FitnessRecord& fitnesses
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(ldebug(), this->GetName() + ": Performing selection.")
        )

    this->SelectNBest(from, into, count, fitnesses, FitnessPred(fitnesses));
}

bool
ElitistSelector::CanSelectSameDesignMoreThanOnce(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return false;
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

ElitistSelector::ElitistSelector(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmSelector(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

ElitistSelector::ElitistSelector(
    const ElitistSelector& copy
    ) :
        GeneticAlgorithmSelector(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

ElitistSelector::ElitistSelector(
    const ElitistSelector& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmSelector(copy, algorithm)
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
