/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class FavorFeasibleOperatorGroup.

    NOTES:

        See notes of FavorFeasibleOperatorGroup.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.1.0

    CHANGES:

        Thu Oct 16 11:29:40 2008 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the FavorFeasibleOperatorGroup class.
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
FavorFeasibleOperatorGroup::Instance(
    )
{
    return FullInstance();
}

inline
const FavorFeasibleOperatorGroup&
FavorFeasibleOperatorGroup::FullInstance(
    )
{
    static const FavorFeasibleOperatorGroup instance;
    return instance;
}

inline
GeneticAlgorithmOperatorRegistry&
FavorFeasibleOperatorGroup::MutatorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
FavorFeasibleOperatorGroup::NichePressureApplicatorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
FavorFeasibleOperatorGroup::ConvergerRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
FavorFeasibleOperatorGroup::CrosserRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
FavorFeasibleOperatorGroup::FitnessAssessorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
FavorFeasibleOperatorGroup::SelectorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
FavorFeasibleOperatorGroup::InitializerRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
FavorFeasibleOperatorGroup::EvaluatorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
FavorFeasibleOperatorGroup::MainLoopRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
FavorFeasibleOperatorGroup::PostProcessorRegistry(
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
