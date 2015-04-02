/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class NullOperatorGroup.

    NOTES:

        See notes of NullOperatorGroup.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 29 10:25:04 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the NullOperatorGroup class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <Mutators/NullMutator.hpp>
#include <Crossers/NullCrosser.hpp>
#include <MainLoops/NullMainLoop.hpp>
#include <Selectors/NullSelector.hpp>
#include <Evaluators/NullEvaluator.hpp>
#include <Convergers/NullConverger.hpp>
#include <Initializers/NullInitializer.hpp>
#include <PostProcessors/NullPostProcessor.hpp>
#include <OperatorGroups/NullOperatorGroup.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <FitnessAssessors/NullFitnessAssessor.hpp>
#include <NichePressureApplicators/NullNichePressureApplicator.hpp>




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

bool NullOperatorGroup::_acquired_operators = false;









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
NullOperatorGroup::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("Null Operator Group");
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
    NullOperatorGroup::Get##optype##Registry( \
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
NullOperatorGroup::GetName(
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


void
NullOperatorGroup::AcquireOperators(
    )
{
    EDDY_FUNC_DEBUGSCOPE

#define REGISTER_METHOD(optype) \
    optype##Registry().register_( \
        Null##optype::Name(), \
        &Null##optype::Create \
        );

    if(!_acquired_operators)
    {
        REGISTER_METHOD(Converger)
        REGISTER_METHOD(Crosser)
        REGISTER_METHOD(Evaluator)
        REGISTER_METHOD(FitnessAssessor)
        REGISTER_METHOD(Initializer)
        REGISTER_METHOD(MainLoop)
        REGISTER_METHOD(Mutator)
        REGISTER_METHOD(Selector)
        REGISTER_METHOD(PostProcessor)
        REGISTER_METHOD(NichePressureApplicator)

        _acquired_operators = true;
    }
}


/*
================================================================================
Structors
================================================================================
*/

NullOperatorGroup::NullOperatorGroup(
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
