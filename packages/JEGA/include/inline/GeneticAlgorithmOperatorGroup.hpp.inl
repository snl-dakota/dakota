/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class GeneticAlgorithmOperatorGroup.

    NOTES:

        See notes of GeneticAlgorithmOperatorGroup.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue May 27 15:29:52 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the GeneticAlgorithmOperatorGroup
 *        class.
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



template <typename FROM, typename INTO>
void
GeneticAlgorithmOperatorGroup::AbsorbOperators(
    )
{
    Absorb(FROM::MutatorRegistry(), INTO::MutatorRegistry());
    Absorb(FROM::ConvergerRegistry(), INTO::ConvergerRegistry());
    Absorb(FROM::CrosserRegistry(), INTO::CrosserRegistry());
    Absorb(FROM::FitnessAssessorRegistry(), INTO::FitnessAssessorRegistry());
    Absorb(
        FROM::NichePressureApplicatorRegistry(),
        INTO::NichePressureApplicatorRegistry()
        );
    Absorb(FROM::SelectorRegistry(), INTO::SelectorRegistry());
    Absorb(FROM::PostProcessorRegistry(), INTO::PostProcessorRegistry());
    Absorb(FROM::InitializerRegistry(), INTO::InitializerRegistry());
    Absorb(FROM::EvaluatorRegistry(), INTO::EvaluatorRegistry());
    Absorb(FROM::MainLoopRegistry(), INTO::MainLoopRegistry());
}



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
