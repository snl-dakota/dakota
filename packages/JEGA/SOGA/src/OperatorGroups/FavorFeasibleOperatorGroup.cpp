/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementaion of class FavorFeasibleOperatorGroup.

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
 * \brief Contains the implementation of the FavorFeasibleOperatorGroup class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utilities/include/EDDY_DebugScope.hpp>
#include <../SOGA/include/Selectors/FavorFeasibleSelector.hpp>
#include <../SOGA/include/OperatorGroups/SOGAOperatorGroup.hpp>
#include <../SOGA/include/OperatorGroups/FavorFeasibleOperatorGroup.hpp>
#include <../SOGA/include/FitnessAssessors/WeightedSumOnlyFitnessAssessor.hpp>



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

bool FavorFeasibleOperatorGroup::_acquired_operators = false;





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
FavorFeasibleOperatorGroup::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("Favor Feasible Operator Group");
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
    FavorFeasibleOperatorGroup::Get##optype##Registry( \
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
FavorFeasibleOperatorGroup::GetName(
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
FavorFeasibleOperatorGroup::AbsorbSOGAOperators(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    SOGAOperatorGroup::Instance();
    AbsorbOperators<SOGAOperatorGroup, FavorFeasibleOperatorGroup>();
    return true;
}

void
FavorFeasibleOperatorGroup::AcquireOperators(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    if(!_acquired_operators)
    {
        AbsorbSOGAOperators();

        FitnessAssessorRegistry().clear();
        SelectorRegistry().clear();
/**
 * \brief A macro that expands to the statement used to register an operator
 *        with the supplied registry of this group.
 *
 * \param reg The registry of this group in which to register \a op
 * \param op The operator to register in the supplied registry \a reg.
 */
#define DO_REGISTER(reg, op) reg().register_(op::Name(), &op::Create)

        DO_REGISTER(FitnessAssessorRegistry, WeightedSumOnlyFitnessAssessor);
        DO_REGISTER(SelectorRegistry, FavorFeasibleSelector);

#undef DO_REGISTER

        _acquired_operators = true;
    }
}




/*
================================================================================
Structors
================================================================================
*/


FavorFeasibleOperatorGroup::FavorFeasibleOperatorGroup(
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
