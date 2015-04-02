/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class DuplicateFreeOperatorGroup.

    NOTES:

        See notes of DuplicateFreeOperatorGroup.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jun 24 09:55:56 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the DuplicateFreeOperatorGroup class.
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

inline
const GeneticAlgorithmOperatorGroup&
DuplicateFreeOperatorGroup::Instance(
    )
{
    return FullInstance();
}

inline
const DuplicateFreeOperatorGroup&
DuplicateFreeOperatorGroup::FullInstance(
    )
{
    static const DuplicateFreeOperatorGroup instance;
    return instance;
}

inline
GeneticAlgorithmOperatorRegistry&
DuplicateFreeOperatorGroup::MutatorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
DuplicateFreeOperatorGroup::NichePressureApplicatorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
DuplicateFreeOperatorGroup::ConvergerRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
DuplicateFreeOperatorGroup::CrosserRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
DuplicateFreeOperatorGroup::FitnessAssessorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
DuplicateFreeOperatorGroup::SelectorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
DuplicateFreeOperatorGroup::InitializerRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
DuplicateFreeOperatorGroup::EvaluatorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
DuplicateFreeOperatorGroup::MainLoopRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
DuplicateFreeOperatorGroup::PostProcessorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
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
