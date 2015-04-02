/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class NullMainLoop.

    NOTES:

        See notes of NullMainLoop.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 29 09:26:34 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the NullMainLoop class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <MainLoops/NullMainLoop.hpp>
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
NullMainLoop::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("null_main_loop");
    return ret;
}

const string&
NullMainLoop::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret(
        "This main loop does nothing.  It is intended to serve as "
        "a null object for main loops.  Use it if you do not wish "
        "to perform a main loop."
        );
    return ret;
}

GeneticAlgorithmOperator*
NullMainLoop::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new NullMainLoop(algorithm);
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
NullMainLoop::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NullMainLoop::Name();
}

string
NullMainLoop::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NullMainLoop::Description();
}

GeneticAlgorithmOperator*
NullMainLoop::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new NullMainLoop(*this, algorithm);
}

bool
NullMainLoop::RunGeneration(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGALOG_II(GetLogger(), ldebug(), this, text_entry(ldebug(),
        GetName() + ": in use."))

    this->IncCurrentGeneration();
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
NullMainLoop::NullMainLoop(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmMainLoop(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

NullMainLoop::NullMainLoop(
    const NullMainLoop& copy
    ) :
        GeneticAlgorithmMainLoop(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

NullMainLoop::NullMainLoop(
    const NullMainLoop& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmMainLoop(copy, algorithm)
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
