/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class DuplicateFreeOperatorGroup.

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
 * \brief Contains the implementation of the DuplicateFreeOperatorGroup class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <Mutators/RandomBitMutator.hpp>
#include <Crossers/NPointBinaryCrosser.hpp>
#include <Mutators/GaussianOffsetMutator.hpp>
#include <OperatorGroups/NullOperatorGroup.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <MainLoops/DuplicateRemovingMainLoop.hpp>
#include <Mutators/LocalDesignVariableMutator.hpp>
#include <OperatorGroups/StandardOperatorGroup.hpp>
#include <Initializers/RandomUniqueInitializer.hpp>
#include <Mutators/RandomDesignVariableMutator.hpp>
#include <OperatorGroups/DuplicateFreeOperatorGroup.hpp>
#include <Crossers/NPointParameterizedBinaryCrosser.hpp>
#include <Selectors/NonDuplicatingRouletteWheelSelector.hpp>

/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;







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

bool DuplicateFreeOperatorGroup::_acquired_operators = false;







/*
================================================================================
Mutators
================================================================================
*/








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
const string&
DuplicateFreeOperatorGroup::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("Duplicate Free Operator Group");
    return ret;
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
#define GET_REGISTRY_METHOD(optype) \
    GeneticAlgorithmOperatorRegistry& \
    DuplicateFreeOperatorGroup::Get##optype##Registry( \
        ) const \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return optype##Registry(); \
    }

GET_REGISTRY_METHOD(Converger)
GET_REGISTRY_METHOD(Crosser)
GET_REGISTRY_METHOD(Evaluator)
GET_REGISTRY_METHOD(FitnessAssessor)
GET_REGISTRY_METHOD(Initializer)
GET_REGISTRY_METHOD(MainLoop)
GET_REGISTRY_METHOD(Mutator)
GET_REGISTRY_METHOD(Selector)
GET_REGISTRY_METHOD(PostProcessor)
GET_REGISTRY_METHOD(NichePressureApplicator)

#undef GET_REGISTRY_METHOD

string
DuplicateFreeOperatorGroup::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return Name();
}



/*
================================================================================
Private Methods
================================================================================
*/

bool
DuplicateFreeOperatorGroup::AbsorbNullOperators(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    NullOperatorGroup::Instance();
    AbsorbOperators<NullOperatorGroup, DuplicateFreeOperatorGroup>();
    return true;
}

void
DuplicateFreeOperatorGroup::AcquireOperators(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    if(!_acquired_operators)
    {

/**
 * \brief A macro that expands to the statement used to register an operator
 *        with the supplied registry of this group.
 *
 * \param reg The registry of this group in which to register \a op
 * \param op The operator to register in the supplied registry \a reg.
 */
#define DO_REGISTER(reg, op) reg().register_(op::Name(), &op::Create)

        // Register all the convergers
        ConvergerRegistry().merge(
            StandardOperatorGroup::ConvergerRegistry()
            );

        // Register all the crossers
        DO_REGISTER(CrosserRegistry, NPointBinaryCrosser);
        DO_REGISTER(CrosserRegistry, NPointParameterizedBinaryCrosser);

        // Register all the Evaluators
        EvaluatorRegistry().merge(
            StandardOperatorGroup::EvaluatorRegistry()
            );

        // Register all the Fitness Assessors

        // Register all the Initializers

        // Register the RandomUniqueInitializer
        DO_REGISTER(InitializerRegistry, RandomUniqueInitializer);

        // Register all the MainLoops

        // Register the DuplicateRemovingMainLoop
        DO_REGISTER(MainLoopRegistry, DuplicateRemovingMainLoop);

        // Register all the Mutators
        DO_REGISTER(MutatorRegistry, LocalDesignVariableMutator);
        DO_REGISTER(MutatorRegistry, RandomDesignVariableMutator);
        DO_REGISTER(MutatorRegistry, RandomBitMutator);
        DO_REGISTER(MutatorRegistry, GaussianOffsetMutator);

        // Register all the Selectors
        DO_REGISTER(SelectorRegistry, NonDuplicatingRouletteWheelSelector);

#undef DO_REGISTER

        AbsorbNullOperators();

        _acquired_operators = true;
    }
}







/*
================================================================================
Structors
================================================================================
*/


DuplicateFreeOperatorGroup::DuplicateFreeOperatorGroup(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    AcquireOperators();
}






/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA
