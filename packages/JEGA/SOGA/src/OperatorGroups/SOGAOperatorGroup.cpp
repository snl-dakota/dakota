/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementaion of class SOGAOperatorGroup.

    NOTES:

        See notes of SOGAOperatorGroup.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Jun 11 11:29:40 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the SOGAOperatorGroup class.
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
#include <OperatorGroups/StandardOperatorGroup.hpp>
#include <Convergers/BestFitnessTrackerConverger.hpp>
#include <Convergers/AverageFitnessTrackerConverger.hpp>
#include <../SOGA/include/FitnessAssessors/ExteriorPenaltyFitnessAssessor.hpp>



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

bool SOGAOperatorGroup::_acquired_operators = false;





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
SOGAOperatorGroup::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("Standard SOGA Operator Group");
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
    SOGAOperatorGroup::Get##optype##Registry( \
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
SOGAOperatorGroup::GetName(
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
SOGAOperatorGroup::AbsorbStandardOperators(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    StandardOperatorGroup::Instance();
    AbsorbOperators<StandardOperatorGroup, SOGAOperatorGroup>();
    return true;
}

void
SOGAOperatorGroup::AcquireOperators(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    if(!_acquired_operators)
    {
        AbsorbStandardOperators();

/**
 * \brief A macro that expands to the statement used to register an operator
 *        with the supplied registry of this group.
 *
 * \param reg The registry of this group in which to register \a op
 * \param op The operator to register in the supplied registry \a reg.
 */
#define DO_REGISTER(reg, op) reg().register_(op::Name(), &op::Create)

        DO_REGISTER(ConvergerRegistry, BestFitnessTrackerConverger);
        DO_REGISTER(ConvergerRegistry, AverageFitnessTrackerConverger);
        DO_REGISTER(FitnessAssessorRegistry, ExteriorPenaltyFitnessAssessor);
        //DO_REGISTER(FitnessAssessorRegistry, WeightedSumOnlyFitnessAssessor);
        //DO_REGISTER(SelectorRegistry, FavorFeasibleSelector);

#undef DO_REGISTER

        _acquired_operators = true;
    }
}




/*
================================================================================
Structors
================================================================================
*/


SOGAOperatorGroup::SOGAOperatorGroup(
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
