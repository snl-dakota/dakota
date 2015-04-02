/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class NullNichePressureApplicator.

    NOTES:

        See notes of NullNichePressureApplicator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Thu Jan 05 14:48:43 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the NullNichePressureApplicator class.
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
#include <NichePressureApplicators/NullNichePressureApplicator.hpp>







/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
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
NullNichePressureApplicator::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("null_niching");
    return ret;
}

const string&
NullNichePressureApplicator::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This niche pressure applicator does nothing.  It is intended to "
        "serve as a null object for niche pressure applicators.  Use it if "
        "you do not wish to apply any niche pressure."
        );
    return ret;
}

GeneticAlgorithmOperator*
NullNichePressureApplicator::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new NullNichePressureApplicator(algorithm);
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
NullNichePressureApplicator::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NullNichePressureApplicator::Name();
}

string
NullNichePressureApplicator::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NullNichePressureApplicator::Description();
}

GeneticAlgorithmOperator*
NullNichePressureApplicator::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new NullNichePressureApplicator(*this, algorithm);
}

void
NullNichePressureApplicator::ApplyNichePressure(
    JEGA::Utilities::DesignGroup&,
    const FitnessRecord&
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








/*
================================================================================
Structors
================================================================================
*/



NullNichePressureApplicator::NullNichePressureApplicator(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmNichePressureApplicator(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

NullNichePressureApplicator::NullNichePressureApplicator(
    const NullNichePressureApplicator& copy
    ) :
        GeneticAlgorithmNichePressureApplicator(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

NullNichePressureApplicator::NullNichePressureApplicator(
    const NullNichePressureApplicator& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmNichePressureApplicator(copy, algorithm)
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

