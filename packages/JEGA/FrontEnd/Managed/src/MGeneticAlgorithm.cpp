/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Managed Front End

    CONTENTS:

        Implementation of class MGeneticAlgorithm.

    NOTES:

        See notes of MGeneticAlgorithm.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Feb 14 16:01:13 2007 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the MGeneticAlgorithm class.
 */




/*
================================================================================
Includes
================================================================================
*/
#include <stdafx.h>
#include <MSolution.hpp>
#include <MGeneticAlgorithm.hpp>
#include <MGeneticAlgorithmEvaluator.hpp>

#pragma unmanaged
#include <../include/GeneticAlgorithm.hpp>
#pragma managed






/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace JEGA::Utilities;
using namespace JEGA::Algorithms;






/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace FrontEnd {
        namespace Managed {





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
GeneticAlgorithm&
MGeneticAlgorithm::Manifest(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_theAlg != 0x0)
    return *this->_theAlg;
}

void
MGeneticAlgorithm::DissociateGA(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_theAlg = 0x0;
}

MSolution MOH
MGeneticAlgorithm::GetBestDesign(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_theAlg != 0x0)
    const Design* best = this->_theAlg->GetBestDesign();
    if(best == 0x0) return MANAGED_NULL_HANDLE;
    return MANAGED_GCNEW MSolution(*best);
}

SolutionVector MOH
MGeneticAlgorithm::GetCurrentSolution(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_theAlg != 0x0)

    DesignOFSortSet solutions(this->_theAlg->GetCurrentSolution());

    SolutionVector MOH ret =
        MANAGED_GCNEW SolutionVector(static_cast<int>(solutions.size()));

    for(DesignOFSortSet::const_iterator it(solutions.begin());
        it!=solutions.end(); ++it) ret->Add(MANAGED_GCNEW MSolution(**it));

    return ret;
}

SolutionVector MOH
MGeneticAlgorithm::GetCurrentPopulation(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_theAlg != 0x0)

    const DesignOFSortSet& solutions =
        this->_theAlg->GetPopulation().GetOFSortContainer();

    SolutionVector MOH ret =
        MANAGED_GCNEW SolutionVector(static_cast<int>(solutions.size()));

    for(DesignOFSortSet::const_iterator it(solutions.begin());
        it!=solutions.end(); ++it) ret->Add(MANAGED_GCNEW MSolution(**it));

    return ret;
}

eddy::utilities::uint64_t
MGeneticAlgorithm::GetGenerationNumber(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return static_cast<eddy::utilities::uint64_t>(this->_theAlg->GetGenerationNumber());
}

eddy::utilities::uint64_t
MGeneticAlgorithm::GetPopulationSize(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return static_cast<eddy::utilities::uint64_t>(this->_theAlg->GetPopulation().SizeOF());
}

eddy::utilities::uint64_t
MGeneticAlgorithm::GetNDV(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return static_cast<eddy::utilities::uint64_t>(this->_theAlg->GetDesignTarget().GetNDV());
}

eddy::utilities::uint64_t
MGeneticAlgorithm::GetNOF(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_theAlg->GetDesignTarget().GetNOF();
}

eddy::utilities::uint64_t
MGeneticAlgorithm::GetNCN(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_theAlg->GetDesignTarget().GetNCN();
}

void
MGeneticAlgorithm::FlushLogStreams(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_theAlg->GetLogger().FlushStreams();
}

MGeneticAlgorithmEvaluator MOH
MGeneticAlgorithm::GetEvaluator(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    if(this->_wrappedEvaler == MANAGED_NULL_HANDLE)
    {
        const GeneticAlgorithmEvaluator* evaler =
            &this->_theAlg->GetOperatorSet().GetEvaluator();

        this->_wrappedEvaler = MANAGED_GCNEW MGeneticAlgorithmEvaluator(evaler);
    }

    return this->_wrappedEvaler;
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
void
MGeneticAlgorithm::MANAGED_DISPOSE(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    // Do not destroy the GA here.  That will be done externally by a driver.
    this->DissociateGA();
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

MGeneticAlgorithm::MGeneticAlgorithm(
    GeneticAlgorithm* wrapped
    ) :
        _theAlg(wrapped),
        _wrappedEvaler(MANAGED_NULL_HANDLE)
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_theAlg != 0x0)
}

MGeneticAlgorithm::~MGeneticAlgorithm(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->MANAGED_DISPOSE();
}









/*
================================================================================
End Namespace
================================================================================
*/
        } // namespace Managed
    } // namespace FrontEnd
} // namespace JEGA

