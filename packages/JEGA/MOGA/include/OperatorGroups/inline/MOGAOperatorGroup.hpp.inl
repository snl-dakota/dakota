/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class MOGAOperatorGroup.

    NOTES:

        See notes of MOGAOperatorGroup.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Jun 11 11:29:41 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the MOGAOperatorGroup class.
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
MOGAOperatorGroup::Instance(
    )
{
    return FullInstance();
}

inline
const MOGAOperatorGroup&
MOGAOperatorGroup::FullInstance(
    )
{
    static const MOGAOperatorGroup instance;
    return instance;
}

inline
GeneticAlgorithmOperatorRegistry&
MOGAOperatorGroup::MutatorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
MOGAOperatorGroup::NichePressureApplicatorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
MOGAOperatorGroup::ConvergerRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
MOGAOperatorGroup::CrosserRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
MOGAOperatorGroup::FitnessAssessorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
MOGAOperatorGroup::SelectorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
MOGAOperatorGroup::InitializerRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
MOGAOperatorGroup::EvaluatorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
MOGAOperatorGroup::MainLoopRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
MOGAOperatorGroup::PostProcessorRegistry(
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
