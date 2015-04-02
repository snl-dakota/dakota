/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation if class NullFitnessAssessor.

    NOTES:

        See notes of NullFitnessAssessor.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 29 09:26:26 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the NullFitnessAssessor class.
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
#include <utilities/include/EDDY_DebugScope.hpp>
#include <FitnessAssessors/NullFitnessAssessor.hpp>








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
NullFitnessAssessor::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("null_fitness");
    return ret;
}

const string&
NullFitnessAssessor::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This fitness assessor does nothing.  It is intended to serve as "
        "a null object for fitness assessors.  Use it if you do not wish "
        "to perform any fitness assessment."
        );
    return ret;
}

GeneticAlgorithmOperator*
NullFitnessAssessor::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new NullFitnessAssessor(algorithm);
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
NullFitnessAssessor::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NullFitnessAssessor::Name();
}

string
NullFitnessAssessor::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NullFitnessAssessor::Description();
}

GeneticAlgorithmOperator*
NullFitnessAssessor::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    return new NullFitnessAssessor(*this, algorithm);
}


const FitnessRecord*
NullFitnessAssessor::AssessFitness(
    const DesignGroupVector&
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGALOG_II(GetLogger(), ldebug(), this, text_entry(ldebug(),
        GetName() + ": in use."))
    return new FitnessRecord();
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

NullFitnessAssessor::NullFitnessAssessor(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmFitnessAssessor(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

NullFitnessAssessor::NullFitnessAssessor(
    const NullFitnessAssessor& copy
    ) :
        GeneticAlgorithmFitnessAssessor(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

NullFitnessAssessor::NullFitnessAssessor(
    const NullFitnessAssessor& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmFitnessAssessor(copy, algorithm)
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
