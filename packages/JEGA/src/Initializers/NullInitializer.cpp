/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class NullInitializer.

    NOTES:

        See notes of NullInitializer.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 29 09:26:18 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the NullInitializer class.
 */


/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/Logging.hpp>
#include <Initializers/NullInitializer.hpp>
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
NullInitializer::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("null_initialization");
    return ret;
}

const string&
NullInitializer::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret(
        "This initializer does nothing.  It is intended to serve as "
        "a null object for initializers.  Use it if you do not wish "
        "to perform any initialization."
        );
    return ret;
}

GeneticAlgorithmOperator*
NullInitializer::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new NullInitializer(algorithm);
}

bool
NullInitializer::CanProduceInvalidVariableValues(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return false;
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
NullInitializer::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NullInitializer::Name();
}

string
NullInitializer::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NullInitializer::Description();
}

GeneticAlgorithmOperator*
NullInitializer::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new NullInitializer(*this, algorithm);
}


void
NullInitializer::Initialize(
    DesignGroup&
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGALOG_II(this->GetLogger(), ldebug(), this, text_entry(ldebug(),
        this->GetName() + ": in use."))
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

NullInitializer::NullInitializer(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmInitializer(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

NullInitializer::NullInitializer(
    const NullInitializer& copy
    ) :
        GeneticAlgorithmInitializer(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

NullInitializer::NullInitializer(
    const NullInitializer& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmInitializer(copy, algorithm)
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
