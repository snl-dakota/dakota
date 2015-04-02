/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implemenation of class ExteriorPenaltyFitnessAssessor.

    NOTES:

        See notes of ExteriorPenaltyFitnessAssessor.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Jul 23 08:18:58 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the ExteriorPenaltyFitnessAssessor
 *        class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/DesignGroup.hpp>
#include <../Utilities/include/DesignValueMap.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignStatistician.hpp>
#include <../Utilities/include/UnconstrainedStatistician.hpp>
#include <../SOGA/include/FitnessAssessors/ExteriorPenaltyFitnessAssessor.hpp>

/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Utilities;







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
ExteriorPenaltyFitnessAssessor::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("merit_function");
    return ret;
}

const string&
ExteriorPenaltyFitnessAssessor::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This fitness assessor specializes the PenaltyFitnessAssessor "
        "for the exterior penalty function formulation."
        );
    return ret;
}

GeneticAlgorithmOperator*
ExteriorPenaltyFitnessAssessor::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new ExteriorPenaltyFitnessAssessor(algorithm);
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

DesignDoubleMap
ExteriorPenaltyFitnessAssessor::ApplyPenalties(
    const DesignGroupVector& groups
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // need an unconstrained statistician to assess penalties
    // prepare to iterate the groups.
    DesignDoubleMap ret(
        UnconstrainedStatistician::ApplyExteriorPenalty(
            groups, this->GetMultiplier()
            )
        );

    return UnconstrainedStatistician::AddOutOfBoundsPenalties(groups, ret);
}

string
ExteriorPenaltyFitnessAssessor::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return ExteriorPenaltyFitnessAssessor::Name();
}

string
ExteriorPenaltyFitnessAssessor::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return ExteriorPenaltyFitnessAssessor::Description();
}

GeneticAlgorithmOperator*
ExteriorPenaltyFitnessAssessor::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new ExteriorPenaltyFitnessAssessor(*this, algorithm);
}







/*
================================================================================
Private Methods
================================================================================
*/








/*
================================================================================
Structors
================================================================================
*/


ExteriorPenaltyFitnessAssessor::ExteriorPenaltyFitnessAssessor(
    GeneticAlgorithm& algorithm
    ) :
        PenaltyFitnessAssessorBase(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

ExteriorPenaltyFitnessAssessor::ExteriorPenaltyFitnessAssessor(
    const ExteriorPenaltyFitnessAssessor& copy
    ) :
        PenaltyFitnessAssessorBase(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

ExteriorPenaltyFitnessAssessor::ExteriorPenaltyFitnessAssessor(
    const ExteriorPenaltyFitnessAssessor& copy,
    GeneticAlgorithm& algorithm
    ) :
        PenaltyFitnessAssessorBase(copy, algorithm)
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
