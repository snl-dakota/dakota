/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class StandardOperatorGroup.

    NOTES:

        See notes of StandardOperatorGroup.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Jun 04 08:43:18 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the StandardOperatorGroup class.
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
StandardOperatorGroup::Instance(
    )
{
    return FullInstance();
}

inline
const StandardOperatorGroup&
StandardOperatorGroup::FullInstance(
    )
{
    static const StandardOperatorGroup instance;
    return instance;
}

inline
GeneticAlgorithmOperatorRegistry&
StandardOperatorGroup::MutatorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
StandardOperatorGroup::NichePressureApplicatorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
StandardOperatorGroup::ConvergerRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
StandardOperatorGroup::CrosserRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
StandardOperatorGroup::FitnessAssessorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
StandardOperatorGroup::SelectorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
StandardOperatorGroup::InitializerRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
StandardOperatorGroup::EvaluatorRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
StandardOperatorGroup::MainLoopRegistry(
    )
{
    static GeneticAlgorithmOperatorRegistry registry;
    return registry;
}

inline
GeneticAlgorithmOperatorRegistry&
StandardOperatorGroup::PostProcessorRegistry(
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
