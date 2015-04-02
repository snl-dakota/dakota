/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class GeneticAlgorithmConverger

    NOTES:

        See notes of GeneticAlgorithmConverger.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue May 20 11:25:12 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the GeneticAlgorithmConverger class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <limits>
#include <GeneticAlgorithm.hpp>
#include <GeneticAlgorithmConverger.hpp>
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
const std::size_t GeneticAlgorithmConverger::DEFAULT_MAX_GENS(
    std::numeric_limits<std::size_t>::max()
    );

const std::size_t GeneticAlgorithmConverger::DEFAULT_MAX_EVALS(
    std::numeric_limits<std::size_t>::max()
    );







/*
================================================================================
Mutators
================================================================================
*/
void
GeneticAlgorithmConverger::SetMaxGenerations(
    std::size_t maxGens
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_maxGens = maxGens;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": Maximum allowable generations now = ")
            << this->_maxGens
        )
}

void
GeneticAlgorithmConverger::SetMaxEvaluations(
    std::size_t maxEvals
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_maxEvals = maxEvals;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(),
            this->GetName() + ": Maximum allowable evaluations now = ")
            << this->_maxEvals
        )
}

void
GeneticAlgorithmConverger::SetConverged(
    bool lval
    )
{
    EDDY_FUNC_DEBUGSCOPE

    this->_converged = lval;

    JEGAIFLOG_CF_II(this->_converged, this->GetLogger(), lverbose(), this,
        text_entry(lverbose(), this->GetName() + ": Convergence attained.")
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
bool
GeneticAlgorithmConverger::IsMaxGensExceeded(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return (this->GetAlgorithm().GetGenerationNumber() > this->_maxGens);
}

bool
GeneticAlgorithmConverger::IsMaxEvalsExceeded(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return (this->GetAlgorithm().GetNumberEvaluations() > this->_maxEvals);
}

bool
GeneticAlgorithmConverger::IsMaxGensReached(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return (this->GetAlgorithm().GetGenerationNumber() >= this->_maxGens);
}

bool
GeneticAlgorithmConverger::IsMaxEvalsReached(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return (this->GetAlgorithm().GetNumberEvaluations() >= this->_maxEvals);
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
bool
GeneticAlgorithmConverger::PollForParameters(
    const JEGA::Utilities::ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Begin by extracting the maximum generations.
    bool success = ParameterExtractor::GetSizeTypeFromDB(
        db, "method.max_iterations", this->_maxGens
        );

    // If we did not find the max iterations, warn about it and use the default
    // value.  Note that if !success, then maxGens is still equal to _maxGens
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The maximum allowable "
            "number of generations was not found in the parameter "
            "database.  Using the current value of ") << this->_maxGens
        )

    // now go ahead and set it.
    this->SetMaxGenerations(this->_maxGens);

    success = ParameterExtractor::GetSizeTypeFromDB(
        db, "method.max_function_evaluations", this->_maxEvals
        );

    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The maximum allowable "
            "number of evaluations was not found in the parameter "
            "database.  Using the current value of ") << this->_maxEvals
        )

    this->SetMaxEvaluations(this->_maxEvals);

    return true;
}

string
GeneticAlgorithmConverger::GetType(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return "Converger";
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
GeneticAlgorithmConverger::GeneticAlgorithmConverger(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmOperator(algorithm),
        _maxGens(DEFAULT_MAX_GENS),
        _maxEvals(DEFAULT_MAX_EVALS),
        _converged(false)
{
    EDDY_FUNC_DEBUGSCOPE
}


GeneticAlgorithmConverger::GeneticAlgorithmConverger(
    const GeneticAlgorithmConverger& copy
    ) :
        GeneticAlgorithmOperator(copy),
        _maxGens(copy._maxGens),
        _maxEvals(copy._maxEvals),
        _converged(copy._converged)
{
    EDDY_FUNC_DEBUGSCOPE
}


GeneticAlgorithmConverger::GeneticAlgorithmConverger(
    const GeneticAlgorithmConverger& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmOperator(copy, algorithm),
        _maxGens(copy._maxGens),
        _maxEvals(copy._maxEvals),
        _converged(copy._converged)
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
