/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class GeneticAlgorithmMutator

    NOTES:

        See notes of GeneticAlgorithmMutator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed May 21 14:40:16 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the GeneticAlgorithmMutator class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utilities/include/Math.hpp>
#include <GeneticAlgorithmMutator.hpp>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>
#include <utilities/include/RandomNumberGenerator.hpp>






/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA;
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
const double GeneticAlgorithmMutator::DEFAULT_RATE(0.05);







/*
================================================================================
Mutators
================================================================================
*/
void
GeneticAlgorithmMutator::SetRate(
    double rate
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGAIFLOG_CF_II(rate < 0.0, this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(), this->GetName() + ": Mutation rate must be a "
            "non-negative number.  Changing provided value of ")
             << rate << " to 0.0."
            );

    this->_rate = Math::Max(rate, 0.0);

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": Mutation rate now = ") << this->_rate
        )

    JEGAIFLOG_CF_II(_rate > 1.0, this->GetLogger(), lquiet(), this,
        text_entry(lquiet(), this->GetName() + ": Mutation rate is > 100%. "
                   " This may be a problem for some mutators.")
        )
}








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








/*
================================================================================
Subclass Visible Methods
================================================================================
*/

vector<DesignDVSortSet::iterator>
GeneticAlgorithmMutator::ChooseDesignsToMutate(
    size_t howMany,
    DesignGroup& from
    )
{
    vector<size_t> mutate_locs(howMany, 0U);

    for(size_t i=0; i<howMany; ++i) mutate_locs[i] =
        RandomNumberGenerator::UniformInt<size_t>(0, from.SizeDV()-1);

    std::sort(mutate_locs.begin(), mutate_locs.end());

    vector<DesignDVSortSet::iterator> toMutate;
    toMutate.reserve(howMany);

    DesignDVSortSet::iterator tmIt(from.BeginDV());
    size_t last = 0;
    for(size_t i=0; i<howMany; ++i)
    {
        if(mutate_locs[i] == last) continue;
        std::advance(tmIt, mutate_locs[i] - last);
        toMutate.push_back(tmIt);
        last = mutate_locs[i];
    }

    return toMutate;
}







/*
================================================================================
Subclass Overridable Methods
================================================================================
*/
bool
GeneticAlgorithmMutator::PollForParameters(
    const JEGA::Utilities::ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    bool success = ParameterExtractor::GetDoubleFromDB(
        db, "method.mutation_rate", this->_rate
        );

    // If we did not find the mutation rate, warn about it and use the default
    // value.  Note that if !success, then _rate has not been altered.
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The mutation rate was "
            "not found in the parameter database.  Using the current value of ")
            << _rate
        )

    // Now, whether or not we were successful in finding it, we will set the
    // value to the current value.  The set method will do the necessary
    // sanity checking and logging.
    this->SetRate(this->_rate);

    return true;
}

string
GeneticAlgorithmMutator::GetType(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return "Mutator";
}

bool
GeneticAlgorithmMutator::CanProduceInvalidVariableValues(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
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
GeneticAlgorithmMutator::GeneticAlgorithmMutator(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmOperator(algorithm),
        _rate(DEFAULT_RATE)
{
    EDDY_FUNC_DEBUGSCOPE
}

GeneticAlgorithmMutator::GeneticAlgorithmMutator(
    const GeneticAlgorithmMutator& copy
    ) :
        GeneticAlgorithmOperator(copy),
        _rate(copy._rate)
{
    EDDY_FUNC_DEBUGSCOPE
}

GeneticAlgorithmMutator::GeneticAlgorithmMutator(
    const GeneticAlgorithmMutator& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmOperator(copy, algorithm),
        _rate(copy._rate)
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
