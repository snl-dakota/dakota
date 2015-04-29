/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class StandardOperatorGroup.

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
 * \brief Contains the implementation of the StandardOperatorGroup class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utilities/include/EDDY_DebugScope.hpp>

#include <Mutators/RandomBitMutator.hpp>
#include <Mutators/CauchyOffsetMutator.hpp>
#include <Mutators/UniformOffsetMutator.hpp>
#include <Mutators/GaussianOffsetMutator.hpp>
#include <Mutators/LocalDesignVariableMutator.hpp>
#include <Mutators/RandomDesignVariableMutator.hpp>

#include <OperatorGroups/NullOperatorGroup.hpp>
#include <OperatorGroups/StandardOperatorGroup.hpp>

#include <Initializers/RandomInitializer.hpp>
#include <Initializers/FlatFileInitializer.hpp>
#include <Initializers/DoubleMatrixInitializer.hpp>
#include <Initializers/RandomUniqueInitializer.hpp>

#include <Crossers/NPointRealCrosser.hpp>
#include <Crossers/NPointBinaryCrosser.hpp>
#include <Crossers/RandomDesignVariableCrosser.hpp>
#include <Crossers/NPointParameterizedBinaryCrosser.hpp>

#include <Convergers/MaxGenEvalConverger.hpp>
#include <Convergers/MaxGenEvalTimeConverger.hpp>
#include <Convergers/MaximumEvaluationConverger.hpp>
#include <Convergers/MaximumGenerationConverger.hpp>
#include <Convergers/MaximumWallClockTimeConverger.hpp>

#include <MainLoops/StandardMainLoop.hpp>
#include <MainLoops/DuplicateRemovingMainLoop.hpp>

#include <Selectors/ElitistSelector.hpp>
#include <Selectors/BelowLimitSelector.hpp>
#include <Selectors/RouletteWheelSelector.hpp>
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
bool StandardOperatorGroup::_acquired_operators = false;





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
StandardOperatorGroup::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("Standard Operator Group");
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

/// A macro to write implemenations for the Get operator registry methods.
/**
 * \param optype The operator registry type for which this is to be the get
 *               method.
 */
#define GET_REGISTRY_METHOD(optype) \
    GeneticAlgorithmOperatorRegistry& \
    StandardOperatorGroup::Get##optype##Registry( \
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

string
StandardOperatorGroup::GetName(
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
StandardOperatorGroup::AbsorbNullOperators(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    NullOperatorGroup::Instance();
    AbsorbOperators<NullOperatorGroup, StandardOperatorGroup>();
    return true;
}

void
StandardOperatorGroup::AcquireOperators(
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
        DO_REGISTER(ConvergerRegistry, MaximumEvaluationConverger);
        DO_REGISTER(ConvergerRegistry, MaximumWallClockTimeConverger);
        DO_REGISTER(ConvergerRegistry, MaximumGenerationConverger);
        DO_REGISTER(ConvergerRegistry, MaxGenEvalConverger);
        DO_REGISTER(ConvergerRegistry, MaxGenEvalTimeConverger);

        // Register all the crossers
        DO_REGISTER(CrosserRegistry, RandomDesignVariableCrosser);
        DO_REGISTER(CrosserRegistry, NPointBinaryCrosser);
        DO_REGISTER(CrosserRegistry, NPointParameterizedBinaryCrosser);
        DO_REGISTER(CrosserRegistry, NPointRealCrosser);

        // Register all the Evaluators

        // Register all the Fitness Assessors

        // Register all the Initializers
        DO_REGISTER(InitializerRegistry, RandomUniqueInitializer);
        DO_REGISTER(InitializerRegistry, RandomInitializer);
        DO_REGISTER(InitializerRegistry, FlatFileInitializer);
        DO_REGISTER(InitializerRegistry, DoubleMatrixInitializer);

        // Register all the MainLoops
        DO_REGISTER(MainLoopRegistry, StandardMainLoop);
        DO_REGISTER(MainLoopRegistry, DuplicateRemovingMainLoop);

        // Register all the Mutators
        DO_REGISTER(MutatorRegistry, LocalDesignVariableMutator);
        DO_REGISTER(MutatorRegistry, RandomDesignVariableMutator);
        DO_REGISTER(MutatorRegistry, RandomBitMutator);
        DO_REGISTER(MutatorRegistry, GaussianOffsetMutator);
        DO_REGISTER(MutatorRegistry, CauchyOffsetMutator);
        DO_REGISTER(MutatorRegistry, UniformOffsetMutator);

        // Register all the Selectors
        DO_REGISTER(SelectorRegistry, ElitistSelector);
        DO_REGISTER(SelectorRegistry, RouletteWheelSelector);
        DO_REGISTER(SelectorRegistry, NonDuplicatingRouletteWheelSelector);
        DO_REGISTER(SelectorRegistry, BelowLimitSelector);

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

StandardOperatorGroup::StandardOperatorGroup(
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
