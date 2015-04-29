/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class NullConverger.

    NOTES:

        See notes of NullConverger.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 29 09:25:59 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the NullConverger class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <Convergers/NullConverger.hpp>
#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>








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
NullConverger::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("null_convergence");
    return ret;
}

const string&
NullConverger::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This converger does nothing.  It is intended to serve as "
        "a null object for convergers.  Use it if you do not wish "
        "to check for convergence."
        );
    return ret;
}

GeneticAlgorithmOperator*
NullConverger::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new NullConverger(algorithm);
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
NullConverger::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NullConverger::Name();
}

string
NullConverger::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NullConverger::Description();
}

GeneticAlgorithmOperator*
NullConverger::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new NullConverger(*this, algorithm);
}

bool
NullConverger::CheckConvergence(
    const DesignGroup&,
    const FitnessRecord&
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->CheckConvergence();
}

bool
NullConverger::CheckConvergence(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this, text_entry(ldebug(),
        this->GetName() + ": in use."))

    // This converger does not converge
    this->SetConverged(false);

    // return not converged.
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
NullConverger::NullConverger(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmConverger(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

NullConverger::NullConverger(
    const NullConverger& copy
    ) :
        GeneticAlgorithmConverger(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

NullConverger::NullConverger(
    const NullConverger& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmConverger(copy, algorithm)
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
