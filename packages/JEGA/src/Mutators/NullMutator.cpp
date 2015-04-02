/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class NullMutator.

    NOTES:

        See notes of NullMutator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 29 09:26:41 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the NullMutator class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <Mutators/NullMutator.hpp>
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
NullMutator::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("null_mutation");
    return ret;
}

const string&
NullMutator::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This mutator does nothing.  It is intended to serve as "
        "a null object for mutators.  Use it if you do not wish "
        "to perform any mutation."
        );
    return ret;
}

GeneticAlgorithmOperator*
NullMutator::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new NullMutator(algorithm);
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
NullMutator::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NullMutator::Name();
}

string
NullMutator::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return NullMutator::Description();
}

GeneticAlgorithmOperator*
NullMutator::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new NullMutator(*this, algorithm);
}

void
NullMutator::Mutate(
    DesignGroup&,
    DesignGroup&
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGALOG_II(this->GetLogger(), ldebug(), this, text_entry(ldebug(),
        this->GetName() + ": in use."))
}

bool
NullMutator::CanProduceInvalidVariableValues(
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
NullMutator::NullMutator(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmMutator(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

NullMutator::NullMutator(
    const NullMutator& copy
    ) :
        GeneticAlgorithmMutator(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

NullMutator::NullMutator(
    const NullMutator& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmMutator(copy, algorithm)
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
