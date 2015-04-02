/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class GeneticAlgorithmInitializer

    NOTES:

        See notes of GeneticAlgorithmInitializer.hpp

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue May 13 14:20:58 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the GeneticAlgorithmInitializer class.
 */






/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmInitializer.hpp>
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
const std::size_t GeneticAlgorithmInitializer::DEFAULT_INIT_SIZE(50);







/*
================================================================================
Mutators
================================================================================
*/
void
GeneticAlgorithmInitializer::SetSize(
    std::size_t size
    )
{
    EDDY_FUNC_DEBUGSCOPE

    this->_size = size;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": Desired initial population size now = "
            ) << this->_size
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
GeneticAlgorithmInitializer::PollForParameters(
    const JEGA::Utilities::ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    bool success = ParameterExtractor::GetSizeTypeFromDB(
        db, "method.population_size", this->_size
        );

    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The desired initial "
            "population size was not found in the parameter database.  Using "
            "the current value of ") << this->_size
        )

    // now go ahead and set it.
    this->SetSize(this->_size);

    return true;
}

string
GeneticAlgorithmInitializer::GetType(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return "Initializer";
}

bool
GeneticAlgorithmInitializer::CanProduceInvalidVariableValues(
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
GeneticAlgorithmInitializer::GeneticAlgorithmInitializer(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmOperator(algorithm),
        _size(DEFAULT_INIT_SIZE)
{
    EDDY_FUNC_DEBUGSCOPE
}

GeneticAlgorithmInitializer::GeneticAlgorithmInitializer(
    const GeneticAlgorithmInitializer& copy
    ) :
        GeneticAlgorithmOperator(copy),
        _size(copy._size)
{
    EDDY_FUNC_DEBUGSCOPE
}

GeneticAlgorithmInitializer::GeneticAlgorithmInitializer(
    const GeneticAlgorithmInitializer& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmOperator(copy, algorithm),
        _size(copy._size)
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
