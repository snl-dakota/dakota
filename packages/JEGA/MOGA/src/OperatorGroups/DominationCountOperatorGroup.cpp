/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementaion of class DominationCountOperatorGroup.

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
 * \brief Contains the implementation of the DominationCountOperatorGroup
 *        class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <Selectors/BelowLimitSelector.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../MOGA/include/OperatorGroups/MOGAOperatorGroup.hpp>
#include <../MOGA/include/OperatorGroups/DominationCountOperatorGroup.hpp>
#include <../MOGA/include/FitnessAssessors/DominationCountFitnessAssessor.hpp>


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

bool DominationCountOperatorGroup::_acquired_operators = false;






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
DominationCountOperatorGroup::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("Domination Count Operator Group");
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
    DominationCountOperatorGroup::Get##optype##Registry( \
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
DominationCountOperatorGroup::GetName(
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
DominationCountOperatorGroup::AbsorbMOGAOperators(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // This ensures that the MOGAOperatorGroup has acquired its operators.
    MOGAOperatorGroup::Instance();

    // This absorbs all the operators of the MOGA into the Dom Count.
    // It is ok that some of the MOGA fitness assessors and selectors
    // cannot be used by this group.  Those registries will be cleared
    // when this group acquires its operators.
    AbsorbOperators<MOGAOperatorGroup, DominationCountOperatorGroup>();

    // return true b/c all is well.
    return true;
}

void
DominationCountOperatorGroup::AcquireOperators(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    if(!_acquired_operators)
    {
        AbsorbMOGAOperators();

        // clear out the fitness assessors and put in only the
        // DominationCountFitnessAssessor
        FitnessAssessorRegistry().clear();
        FitnessAssessorRegistry().register_(
            DominationCountFitnessAssessor::Name(),
            &DominationCountFitnessAssessor::Create
            );

        // clear out the selector registry and put in only the
        // BelowLimitSelector
        SelectorRegistry().clear();
        SelectorRegistry().register_(
            BelowLimitSelector::Name(),
            &BelowLimitSelector::Create
            );

        _acquired_operators = true;
    }
}








/*
================================================================================
Structors
================================================================================
*/



DominationCountOperatorGroup::DominationCountOperatorGroup(
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
