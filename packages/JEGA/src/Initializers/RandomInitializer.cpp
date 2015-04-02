/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implemenation of class RandomInitializer.

    NOTES:

        See notes of RandomInitializer.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Jun 04 08:04:48 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the RandomInitializer class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/Logging.hpp>
#include <Initializers/RandomInitializer.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <../Utilities/include/DesignTarget.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>







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
RandomInitializer::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("random");
    return ret;
}

const string&
RandomInitializer::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret(
        "This initializer creates the required number of initial "
        "Designs randomly without consideration for uniqueness.  "
        "Variable values are chosen to be in bounds using a uniform "
        "random number distribution.\n\n"
        "The supplied initial size is the number of designs that "
        "will be created."
        );
    return ret;
}

GeneticAlgorithmOperator*
RandomInitializer::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new RandomInitializer(algorithm);
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
RandomInitializer::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    return RandomInitializer::Name();
}

string
RandomInitializer::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    return RandomInitializer::Description();
}

GeneticAlgorithmOperator*
RandomInitializer::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    return new RandomInitializer(*this, algorithm);
}


void
RandomInitializer::Initialize(
    DesignGroup& into
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(ldebug(), this->GetName() + ": Performing initialization.")
        )

    // get the target for the designs we create.
    DesignTarget& target = into.GetDesignTarget();

    // prepare a Design pointer for repeated use.
    Design* newdes = static_cast<Design*>(0);

    JEGA_LOGGING_IF_ON(const std::size_t prevSize = into.GetSize();)

    // get the desired size for the group "into".
    const std::size_t size = this->GetSize();

    // get the number of design variables per Design
    const size_t ndv = target.GetNDV();

    // extract the design variable information
    const DesignVariableInfoVector& dvis =
        target.GetDesignVariableInfos();

    // loop through and create the Designs
    for(size_t i=0; i<size; ++i)
    {
        // Get a new Design class object from the Genetic Algorithm.
        newdes = target.GetNewDesign();

        // create random values for each design variable
        for(size_t j=0; j<ndv; j++)
            newdes->SetVariableRep(j, dvis[j]->GetRandomDoubleRep());

        // put the new Design into the population.
        into.Insert(newdes);
    }

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": ")
            << (into.GetSize() - prevSize) << " random designs created."
        )
}

bool
RandomInitializer::CanProduceInvalidVariableValues(
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
RandomInitializer::RandomInitializer(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmInitializer(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

RandomInitializer::RandomInitializer(
    const RandomInitializer& copy
    ) :
        GeneticAlgorithmInitializer(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

RandomInitializer::RandomInitializer(
    const RandomInitializer& copy,
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
