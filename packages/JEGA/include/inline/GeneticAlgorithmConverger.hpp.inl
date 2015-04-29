/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class GeneticAlgorithmConverger

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
 * \brief Contains the inline methods of the GeneticAlgorithmConverger class.
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
Inline Mutators
================================================================================
*/




/*
================================================================================
Inline Accessors
================================================================================
*/
inline
std::size_t
GeneticAlgorithmConverger::GetMaxGenerations(
    ) const
{
    return this->_maxGens;
}

inline
std::size_t
GeneticAlgorithmConverger::GetMaxEvaluations(
    ) const
{
    return this->_maxEvals;
}

inline
double
GeneticAlgorithmConverger::GetMaxTime(
    ) const
{
    return this->_maxTime;
}

inline
bool
GeneticAlgorithmConverger::GetConverged(
    ) const
{
    return this->_converged;
}






/*
================================================================================
Inline Public Methods
================================================================================
*/










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
