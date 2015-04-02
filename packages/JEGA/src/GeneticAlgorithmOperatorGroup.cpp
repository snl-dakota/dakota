/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class GeneticAlgorithmOperatorGroup.

    NOTES:

        See notes of GeneticAlgorithmOperatorGroup.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue May 27 15:29:52 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the GeneticAlgorithmOperatorGroup
 *        class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmMutator.hpp>
#include <GeneticAlgorithmCrosser.hpp>
#include <GeneticAlgorithmMainLoop.hpp>
#include <GeneticAlgorithmSelector.hpp>
#include <GeneticAlgorithmConverger.hpp>
#include <GeneticAlgorithmEvaluator.hpp>
#include <GeneticAlgorithmInitializer.hpp>
#include <GeneticAlgorithmOperatorSet.hpp>
#include <../Utilities/include/Logging.hpp>
#include <GeneticAlgorithmOperatorGroup.hpp>
#include <GeneticAlgorithmPostProcessor.hpp>
#include <GeneticAlgorithmFitnessAssessor.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <GeneticAlgorithmNichePressureApplicator.hpp>



/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Logging;







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








/*
================================================================================
Public Methods
================================================================================
*/
bool
GeneticAlgorithmOperatorGroup::ContainsSet(
    const GeneticAlgorithmOperatorSet& set
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

#define CONTAINS_TEST(optype) \
    if(!Has##optype(set.Get##optype())) \
    { \
        JEGA_LOGGING_IF_ON( \
        OutputMatchFailure(#optype, set.Get##optype()) \
            ); \
        return false; \
    } \
    else OutputMatchSuccess(#optype, set.Get##optype());

    CONTAINS_TEST(Converger)
    CONTAINS_TEST(Crosser)
    CONTAINS_TEST(FitnessAssessor)
    CONTAINS_TEST(Initializer)
    CONTAINS_TEST(MainLoop)
    CONTAINS_TEST(Mutator)
    CONTAINS_TEST(Selector)
    CONTAINS_TEST(PostProcessor)
    CONTAINS_TEST(NichePressureApplicator)

#undef CONTAINS_TEST

    // if we made it here, we succeeded in matching all operators.
    return true;
}

#define HAS_OP_METHOD(optype) \
    bool \
    GeneticAlgorithmOperatorGroup::Has##optype( \
        const GeneticAlgorithm##optype& op \
        ) const \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return Get##optype##Registry().is_registered(op.GetName()); \
    }

HAS_OP_METHOD(Converger)
HAS_OP_METHOD(Crosser)
HAS_OP_METHOD(FitnessAssessor)
HAS_OP_METHOD(Initializer)
HAS_OP_METHOD(Evaluator)
HAS_OP_METHOD(MainLoop)
HAS_OP_METHOD(Mutator)
HAS_OP_METHOD(Selector)
HAS_OP_METHOD(PostProcessor)
HAS_OP_METHOD(NichePressureApplicator)

#undef HAS_OP_METHOD

#define GET_OP_METHOD(optype) \
    GeneticAlgorithm##optype* \
    GeneticAlgorithmOperatorGroup::Get##optype( \
        const string& name, \
        GeneticAlgorithm& algorithm \
        ) const \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return static_cast<GeneticAlgorithm##optype*>( \
        GetOperator(name, Get##optype##Registry(), algorithm)); \
    }

GET_OP_METHOD(Converger)
GET_OP_METHOD(Crosser)
GET_OP_METHOD(FitnessAssessor)
GET_OP_METHOD(Initializer)
GET_OP_METHOD(Evaluator)
GET_OP_METHOD(MainLoop)
GET_OP_METHOD(Mutator)
GET_OP_METHOD(Selector)
GET_OP_METHOD(PostProcessor)
GET_OP_METHOD(NichePressureApplicator)

#undef GET_OP_METHOD

std::string
GeneticAlgorithmOperatorGroup::GetOperatorReadout(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    std::ostringstream ostr;
    PrintOperators(ostr);
    return ostr.str();
}

void
GeneticAlgorithmOperatorGroup::PrintOperators(
    ostream& stream
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    string prefix("     ");

    stream << "The operators of \"" << GetName() << "\":\n\n";

#define PRINT_OPS(optype) \
    stream << #optype "s:\n"; \
    PrintOps(Get##optype##Registry(), prefix, stream); \
    stream << "\n\n";

    PRINT_OPS(Converger)
    PRINT_OPS(Crosser)
    PRINT_OPS(FitnessAssessor)
    PRINT_OPS(Initializer)
    PRINT_OPS(Evaluator)
    PRINT_OPS(MainLoop)
    PRINT_OPS(Mutator)
    PRINT_OPS(Selector)
    PRINT_OPS(PostProcessor)
    PRINT_OPS(NichePressureApplicator)

#undef PRINT_OPS
}

void
GeneticAlgorithmOperatorGroup::PrintOperators(
    const GeneticAlgorithmOperatorRegistry& reg,
    ostream& stream
    )
{
    EDDY_FUNC_DEBUGSCOPE
    PrintOps(reg, string(), stream);
}






/*
================================================================================
Subclass Visible Methods
================================================================================
*/


GeneticAlgorithmOperator*
GeneticAlgorithmOperatorGroup::GetOperator(
    const string& name,
    const GeneticAlgorithmOperatorRegistry& from,
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    GeneticAlgorithmOperatorRegistry::const_iterator it(from.find(name));
    return (it == from.end()) ? 0x0 : (*it).second(algorithm);
}






/*
================================================================================
Subclass Overridable Methods
================================================================================
*/








/*
================================================================================
Private Methods
================================================================================
*/

void
GeneticAlgorithmOperatorGroup::Absorb(
    const GeneticAlgorithmOperatorRegistry& from,
    GeneticAlgorithmOperatorRegistry& into
    )
{
    EDDY_FUNC_DEBUGSCOPE
    /// The keyed_registry::merge method will do just what we want.
    into.merge(from);
}

void
GeneticAlgorithmOperatorGroup::PrintOps(
    const GeneticAlgorithmOperatorRegistry& reg,
    const string& prefix,
    ostream& stream
    )
{
    EDDY_FUNC_DEBUGSCOPE

    GeneticAlgorithmOperatorRegistry::const_iterator it(reg.begin());

    for(; it!=reg.end(); ++it)
        stream << prefix << (*it).first << "\n";
}

void
GeneticAlgorithmOperatorGroup::OutputMatchFailure(
    const std::string& JEGA_LOGGING_IF_ON(opType),
    const GeneticAlgorithmOperator& JEGA_LOGGING_IF_ON(op)
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    JEGALOG_II_G(lverbose(), this,
        text_entry(lverbose(), GetName() + ": Does not contain " + opType +
                   " \"" + op.GetName() + "\".  Match failed.")
        )
}

void
GeneticAlgorithmOperatorGroup::OutputMatchSuccess(
    const std::string& opType,
    const GeneticAlgorithmOperator& op
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    JEGALOG_II_G(lverbose(), this,
        text_entry(lverbose(), GetName() + ": Contains " + opType +
                   " \"" + op.GetName() + "\".")
        )
}






/*
================================================================================
Structors
================================================================================
*/

GeneticAlgorithmOperatorGroup::~GeneticAlgorithmOperatorGroup(
    )
{
    EDDY_FUNC_DEBUGSCOPE
}






/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA
