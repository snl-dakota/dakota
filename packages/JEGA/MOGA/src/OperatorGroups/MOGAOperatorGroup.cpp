/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementaion of class MOGAOperatorGroup.

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

        Wed Jun 11 11:29:40 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the MOGAOperatorGroup class.
 */


/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utilities/include/EDDY_DebugScope.hpp>
#include <OperatorGroups/StandardOperatorGroup.hpp>
#include <../MOGA/include/Convergers/MOGAConverger.hpp>
#include <../MOGA/include/OperatorGroups/MOGAOperatorGroup.hpp>
#include <../MOGA/include/FitnessAssessors/LayerFitnessAssessor.hpp>
#include <../MOGA/include/PostProcessors/DistanceNichingPostProcessor.hpp>
#include <../MOGA/include/FitnessAssessors/DominationCountFitnessAssessor.hpp>
#include <../MOGA/include/NichePressureApplicators/RadialNichePressureApplicator.hpp>
#include <../MOGA/include/NichePressureApplicators/DistanceNichePressureApplicator.hpp>
#include <../MOGA/include/NichePressureApplicators/MaxDesignsNichePressureApplicator.hpp>


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

bool MOGAOperatorGroup::_acquired_operators = false;

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
MOGAOperatorGroup::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("Standard MOGA Operator Group");
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
    MOGAOperatorGroup::Get##optype##Registry( \
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
MOGAOperatorGroup::GetName(
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
MOGAOperatorGroup::AbsorbStandardOperators(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // This ensures that the StandardOperatorGroup has acquired its operators.
    StandardOperatorGroup::Instance();

    // This absorbs all the operators of the standard into the MOGA.
    AbsorbOperators<StandardOperatorGroup, MOGAOperatorGroup>();

    // Return true b/c all is well.
    return true;
}

void
MOGAOperatorGroup::AcquireOperators(
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

        DO_REGISTER(FitnessAssessorRegistry, LayerFitnessAssessor);
        DO_REGISTER(FitnessAssessorRegistry, DominationCountFitnessAssessor);
        DO_REGISTER(ConvergerRegistry, MOGAConverger);
        DO_REGISTER(
            NichePressureApplicatorRegistry, RadialNichePressureApplicator
            );
        DO_REGISTER(
            NichePressureApplicatorRegistry, DistanceNichePressureApplicator
            );
        DO_REGISTER(
            NichePressureApplicatorRegistry, MaxDesignsNichePressureApplicator
            );
        DO_REGISTER(PostProcessorRegistry, DistanceNichingPostProcessor);

#undef DO_REGISTER

        AbsorbStandardOperators();

        _acquired_operators = true;
    }
}



/*
================================================================================
Structors
================================================================================
*/


MOGAOperatorGroup::MOGAOperatorGroup(
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
