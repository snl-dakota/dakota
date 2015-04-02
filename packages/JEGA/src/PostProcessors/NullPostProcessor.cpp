/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class NullPostProcessor.

    NOTES:

        See notes of NullPostProcessor.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Thu Aug 17 12:57:32 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the NullPostProcessor class.
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
#include <PostProcessors/NullPostProcessor.hpp>






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
NullPostProcessor::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("null_postprocessor");
    return ret;
}

const string&
NullPostProcessor::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret(
        "This post processor does nothing.  It is intended to serve as "
        "a null object for post processors.  Use it if you do not wish "
        "to perform any post processing."
        );
    return ret;
}

GeneticAlgorithmOperator*
NullPostProcessor::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new NullPostProcessor(algorithm);
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
NullPostProcessor::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NullPostProcessor::Name();
}

string
NullPostProcessor::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NullPostProcessor::Description();
}

GeneticAlgorithmOperator*
NullPostProcessor::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new NullPostProcessor(*this, algorithm);
}


void
NullPostProcessor::PostProcess(
    DesignGroup&
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGALOG_II(GetLogger(), ldebug(), this, text_entry(ldebug(),
        GetName() + ": in use."))
}








/*
================================================================================
Private Methods
================================================================================
*/

NullPostProcessor::NullPostProcessor(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmPostProcessor(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

NullPostProcessor::NullPostProcessor(
    const NullPostProcessor& copy
    ) :
        GeneticAlgorithmPostProcessor(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

NullPostProcessor::NullPostProcessor(
    const NullPostProcessor& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmPostProcessor(copy, algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}








/*
================================================================================
Structors
================================================================================
*/








/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA

