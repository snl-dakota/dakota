/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class DominationCountOperatorGroup.

    NOTES:

        See notes of DominationCountOperatorGroup.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Jun 25 07:42:57 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the DominationCountOperatorGroup
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
inline
const GeneticAlgorithmOperatorGroup&
DominationCountOperatorGroup::Instance(
    )
{
    return FullInstance();
}

inline
const DominationCountOperatorGroup&
DominationCountOperatorGroup::FullInstance(
    )
{
    static const DominationCountOperatorGroup instance;
    return instance;
}

inline
GeneticAlgorithmOperatorRegistry&
DominationCountOperatorGroup::MutatorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
DominationCountOperatorGroup::NichePressureApplicatorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
DominationCountOperatorGroup::ConvergerRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
DominationCountOperatorGroup::CrosserRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
DominationCountOperatorGroup::FitnessAssessorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
DominationCountOperatorGroup::SelectorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
DominationCountOperatorGroup::InitializerRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
DominationCountOperatorGroup::EvaluatorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
DominationCountOperatorGroup::MainLoopRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
DominationCountOperatorGroup::PostProcessorRegistry(
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
