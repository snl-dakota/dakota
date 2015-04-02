/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class AllOperators.

    NOTES:

        See notes of AllOperators.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Jun 04 09:20:13 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the AllOperators class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <OperatorGroups/AllOperators.hpp>
#include <OperatorGroups/NullOperatorGroup.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <OperatorGroups/StandardOperatorGroup.hpp>
#include <OperatorGroups/DuplicateFreeOperatorGroup.hpp>
#include <../MOGA/include/OperatorGroups/MOGAOperatorGroup.hpp>
#include <../SOGA/include/OperatorGroups/SOGAOperatorGroup.hpp>
#include <../SOGA/include/OperatorGroups/FavorFeasibleOperatorGroup.hpp>
#include <../MOGA/include/OperatorGroups/DominationCountOperatorGroup.hpp>

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
bool AllOperators::_acquired_operators = false;





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
AllOperators::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("All Operators");
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
    AllOperators::Get##optype##Registry( \
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
AllOperators::GetName(
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
#define ABSORB_METHOD(gpname) \
    bool \
    AllOperators::Absorb##gpname##Operators( \
        ) \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        gpname##OperatorGroup::Instance(); \
        AbsorbOperators<gpname##OperatorGroup, AllOperators>(); \
        return true; \
    }

ABSORB_METHOD(Null)
ABSORB_METHOD(Standard)
ABSORB_METHOD(MOGA)
ABSORB_METHOD(SOGA)
ABSORB_METHOD(DuplicateFree)
ABSORB_METHOD(DominationCount)
ABSORB_METHOD(FavorFeasible)

void
AllOperators::AcquireOperators(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    if(!_acquired_operators)
    {
        AbsorbNullOperators();
        AbsorbStandardOperators();
        AbsorbDuplicateFreeOperators();
        AbsorbMOGAOperators();
        AbsorbDominationCountOperators();
        AbsorbSOGAOperators();
        AbsorbFavorFeasibleOperators();

        _acquired_operators = true;
    }
}



/*
================================================================================
Structors
================================================================================
*/

AllOperators::AllOperators(
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
