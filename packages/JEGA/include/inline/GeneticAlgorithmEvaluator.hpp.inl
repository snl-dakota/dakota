/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class GeneticAlgorithmEvaluator.

    NOTES:

        See notes of GeneticAlgorithmEvaluator.hpp

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue May 20 10:41:52 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the GeneticAlgorithmEvaluator class.
 */





/*
================================================================================
Includes
================================================================================
*/








/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Algorithms {




/*
================================================================================
Nested Utility Class Implementations
================================================================================
*//*
inline
void
GeneticAlgorithmEvaluator::EvaluationJob::SetResult(
    bool res
    )
{
    this->_result = res;
}

inline
JEGA::Utilities::Design&
GeneticAlgorithmEvaluator::EvaluationJob::GetDesign(
    ) const
{
    return this->_toEval;
}

inline
std::size_t
GeneticAlgorithmEvaluator::EvaluationJob::GetEvaluationNumber(
    ) const
{
    return this->_evalNum;
}

inline
GeneticAlgorithmEvaluator&
GeneticAlgorithmEvaluator::EvaluationJob::GetEvaluator(
    )
{
    return this->_evaler;
}

inline
bool
GeneticAlgorithmEvaluator::EvaluationJob::GetResult(
    ) const
{
    return this->_result;
}*/



/*
================================================================================
Inline Mutators
================================================================================
*/
inline
std::size_t
GeneticAlgorithmEvaluator::GetEvaluationConcurrency(
    ) const
{
    return this->_evalConcur;
}









/*
================================================================================
Inline Accessors
================================================================================
*/
inline
std::size_t
GeneticAlgorithmEvaluator::GetMaxEvaluations(
    ) const
{
    return this->_maxEvals;
}


inline
const JEGA::Utilities::DesignDVSortSet&
GeneticAlgorithmEvaluator::GetInjections(
    ) const
{
    return this->_injections;
}





/*
================================================================================
Inline Public Methods
================================================================================
*/


inline
bool
GeneticAlgorithmEvaluator::IsMaxEvalsExceeded(
    ) const
{
    return (this->_numEvals > this->_maxEvals);
}

inline
bool
GeneticAlgorithmEvaluator::IsMaxEvalsReached(
    ) const
{
    return (this->_numEvals >= this->_maxEvals);
}







/*
================================================================================
Inline Subclass Visible Methods
================================================================================
*/








/*
================================================================================
Inline Private Methods
================================================================================
*/








/*
================================================================================
Inline Structors
================================================================================
*/








/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA
