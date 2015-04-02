/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class NullCrosser.

    NOTES:

        See notes of NullCrosser.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 29 09:26:06 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the NullCrosser class.
 */


/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <Crossers/NullCrosser.hpp>
#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>







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
NullCrosser::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("null_crossover");
    return ret;
}

const string&
NullCrosser::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret(
        "This crosser does nothing.  It is intended to serve as "
        "a null object for crossers.  Use it if you do not wish "
        "to perform any crossover."
        );
    return ret;
}

GeneticAlgorithmOperator*
NullCrosser::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new NullCrosser(algorithm);
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
NullCrosser::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NullCrosser::Name();
}

string
NullCrosser::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NullCrosser::Description();
}

GeneticAlgorithmOperator*
NullCrosser::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new NullCrosser(*this, algorithm);
}

void
NullCrosser::Crossover(
    const DesignGroup&,
    DesignGroup&
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGALOG_II(GetLogger(), ldebug(), this, text_entry(ldebug(),
        GetName() + ": in use."))
}

bool
NullCrosser::CanProduceInvalidVariableValues(
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

NullCrosser::NullCrosser(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmCrosser(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

NullCrosser::NullCrosser(
    const NullCrosser& copy
    ) :
        GeneticAlgorithmCrosser(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

NullCrosser::NullCrosser(
    const NullCrosser& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmCrosser(copy, algorithm)
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
