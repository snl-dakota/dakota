/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class GeneticAlgorithmMainLoop

    NOTES:

        See notes of GeneticAlgorithmMainLoop.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed May 21 15:26:50 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the GeneticAlgorithmMainLoop class.
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
inline
void
GeneticAlgorithmMainLoop::SetCurrentGeneration(
    eddy::utilities::uint64_t num
    )
{
    _currGen = num;
}







/*
================================================================================
Inline Accessors
================================================================================
*/
inline
eddy::utilities::uint64_t
GeneticAlgorithmMainLoop::GetCurrentGeneration(
    ) const
{
    return this->_currGen;
}







/*
================================================================================
Inline Public Methods
================================================================================
*/
void
GeneticAlgorithmMainLoop::IncCurrentGeneration(
    eddy::utilities::uint64_t by
    )
{
    this->_currGen += by;
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
