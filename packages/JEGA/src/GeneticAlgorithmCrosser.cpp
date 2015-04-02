/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class GeneticAlgorithmCrosser

    NOTES:

        See notes of GeneticAlgorithmCrosser.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue May 20 13:32:42 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the GeneticAlgorithmCrosser class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utilities/include/Math.hpp>
#include <GeneticAlgorithmCrosser.hpp>
#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>





/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA;
using namespace JEGA::Logging;
using namespace JEGA::Utilities;
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
const double GeneticAlgorithmCrosser::DEFAULT_RATE(0.75);







/*
================================================================================
Mutators
================================================================================
*/
void
GeneticAlgorithmCrosser::SetRate(
    double rate
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGAIFLOG_CF_II(rate < 0.0, this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(), this->GetName() + ": Crossover rate must be a "
            "non-negative number.  Changing provided value of ") << rate
            << " to 0.0."
        )

    this->_rate = Math::Max(rate, 0.0);

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": Crossover rate now = ") << _rate
        )

    JEGAIFLOG_CF_II(_rate > 1.0, this->GetLogger(), lquiet(), this,
        text_entry(lquiet(), this->GetName() + ": Crossover rate is > 100%.  "
            "This may be a problem for some crossers.")
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






/*
================================================================================
Subclass Overridable Methods
================================================================================
*/
bool
GeneticAlgorithmCrosser::PollForParameters(
    const JEGA::Utilities::ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    bool success = ParameterExtractor::GetDoubleFromDB(
        db, "method.crossover_rate", this->_rate
        );

    // If we did not find the crossover rate, warn about it and use the default
    // value.  Note that if !success, then _rate has not been altered.
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The crossover rate was "
            "not found in the parameter database.  Using the current value of ")
            << this->_rate
        )

    // Now, whether or not we were successful in finding it, we will set the
    // value to the current value.  The set method will do the necessary
    // sanity checking and logging.
    this->SetRate(this->_rate);

    return true;
}

string
GeneticAlgorithmCrosser::GetType(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return "Crosser";
}

bool
GeneticAlgorithmCrosser::CanProduceInvalidVariableValues(
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
GeneticAlgorithmCrosser::GeneticAlgorithmCrosser(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmOperator(algorithm),
        _rate(DEFAULT_RATE)
{
    EDDY_FUNC_DEBUGSCOPE
}


GeneticAlgorithmCrosser::GeneticAlgorithmCrosser(
    const GeneticAlgorithmCrosser& copy
    ) :
        GeneticAlgorithmOperator(copy),
        _rate(copy._rate)
{
    EDDY_FUNC_DEBUGSCOPE
}


GeneticAlgorithmCrosser::GeneticAlgorithmCrosser(
    const GeneticAlgorithmCrosser& copy,
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
