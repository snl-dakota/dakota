/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implemenation of class RandomUniqueInitializer.

    NOTES:

        See notes of RandomUniqueInitializer.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Jun 04 08:04:34 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the RandomUniqueInitializer class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <../Utilities/include/DesignTarget.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <Initializers/RandomUniqueInitializer.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>

/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Utilities;
using namespace JEGA::Logging;
using namespace eddy::utilities;








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
RandomUniqueInitializer::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("unique_random");
    return ret;
}

const string&
RandomUniqueInitializer::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This initializer creates a group of unique initial Designs.\n\n"
        "It does so by testing each newly created design against those "
        "that already exist.  Any that are not unique to all others are "
        "discarded.\n\n"
        "Variable values are chosen to be in bounds using a uniform "
        "random number distribution.\n\n"
        "The supplied initial size is the number of designs that "
        "this operator will try to create.  It may be the case that "
        "there are not enough possibilities (combinatorially speaking) "
        "for this operator to complete its task.  It does not explicitly "
        "check for that case.  Instead, it stops trying after it has "
        "failed for max(100, \"initial size\") consecutive creations."
        );
    return ret;
}

GeneticAlgorithmOperator*
RandomUniqueInitializer::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE

    return new RandomUniqueInitializer(algorithm);
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
RandomUniqueInitializer::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return RandomUniqueInitializer::Name();
}

string
RandomUniqueInitializer::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return RandomUniqueInitializer::Description();
}

GeneticAlgorithmOperator*
RandomUniqueInitializer::Clone(
    GeneticAlgorithm& algorithm) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new RandomUniqueInitializer(*this, algorithm);
}


void
RandomUniqueInitializer::Initialize(
    DesignGroup& into
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(GetLogger(), ldebug(), this,
        text_entry(ldebug(), GetName() + ": Performing initialization.")
        )

    // get the target for the designs we create.
    DesignTarget& target = into.GetDesignTarget();

    // prepare a Design pointer for use in the loops below.
    Design* newdes = static_cast<Design*>(0);

    // Get the number of design variables per Design
    const size_t ndv = target.GetNDV();

    // find out how many designs to create.
    const std::size_t size = GetSize();

    JEGA_LOGGING_IF_ON(const std::size_t prevSize = into.GetSize();)

    // keep track of the number of consecutive failures,
    // stop if reaches the desired population size.
    size_t cfs = 0;

    // compute the number of consecutive failures to tolerate.
    const std::size_t ncf = Math::Max<std::size_t>(100, size);

    // extract the design variable information
    const DesignVariableInfoVector& dvis =
        target.GetDesignVariableInfos();

    // loop through and create the Designs

    for(size_t i=0; i<size; ++i)
    {
        // prepare an iterator for repeated use.
        DesignDVSortSet::const_iterator clone;

        // get a new, empty Design to fill up.
        newdes = target.GetNewDesign();

        // this loop asserts the uniqueness of
        // the Designs generated within it.
        do {
            // if it seems that we will be unable
            // to fill the population, give up
            if(cfs++ >= ncf) {

                JEGALOG_II(GetLogger(), lquiet(), this,
                    ostream_entry(lverbose(), GetName() +
                    ": Failed to create the desired number of initial "
                    "designs (")
                    << size << "). Created a total of "
                    << (into.GetSize() - prevSize) << '.'
                    )

                target.TakeDesign(newdes);
                return;
            }

            // Generate a new design randomly.
            for(size_t j=0; j<ndv; j++)
                newdes->SetVariableRep(j, dvis[j]->GetRandomDoubleRep());

            // Test to be sure that the new Design
            // is unique in the population.
            clone = into.GetDVSortContainer().find(newdes);

        // do this for as long as "newdes" is not unique.
        } while (clone != into.EndDV());

        // When we get here, we have a Design that is unique to the population.
        cfs = 0;

        // put the new design into the population.
        into.Insert(newdes);
    }

    JEGALOG_II(GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), GetName() + ": ")
            << (into.GetSize() - prevSize) << " unique random designs created."
        )
}

bool
RandomUniqueInitializer::CanProduceInvalidVariableValues(
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
RandomUniqueInitializer::RandomUniqueInitializer(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmInitializer(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

RandomUniqueInitializer::RandomUniqueInitializer(
    const RandomUniqueInitializer& copy
    ) :
        GeneticAlgorithmInitializer(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

RandomUniqueInitializer::RandomUniqueInitializer(
    const RandomUniqueInitializer& copy,
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
