/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Managed Front End

    CONTENTS:

        Implementation of class MGeneticAlgorithmEvaluator.

    NOTES:

        See notes of MGeneticAlgorithmEvaluator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Feb 23 15:08:29 2009 - Original Version ()

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the MGeneticAlgorithmEvaluator class.
 */




/*
================================================================================
Includes
================================================================================
*/
#include <stdafx.h>
#include <MGeneticAlgorithmEvaluator.hpp>

#pragma unmanaged
#include <GeneticAlgorithmEvaluator.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#pragma managed






/*
================================================================================
Namespace Using Directives
================================================================================
*/








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

eddy::utilities::uint64_t
MGeneticAlgorithmEvaluator::GetNumberEvaluations(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_evaler != 0x0)
    return this->_evaler->GetNumberEvaluations();
}

eddy::utilities::uint64_t
MGeneticAlgorithmEvaluator::GetMaxEvaluations(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_evaler != 0x0)
    return this->_evaler->GetMaxEvaluations();
}

eddy::utilities::uint64_t
MGeneticAlgorithmEvaluator::GetEvaluationConcurrency(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_evaler != 0x0)
    return this->_evaler->GetEvaluationConcurrency();
}







/*
================================================================================
Public Methods
================================================================================
*/

bool
MGeneticAlgorithmEvaluator::IsMaxEvalsExceeded(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_evaler != 0x0)
    return this->_evaler->IsMaxEvalsExceeded();
}

bool
MGeneticAlgorithmEvaluator::IsMaxEvalsReached(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_evaler != 0x0)
    return this->_evaler->IsMaxEvalsReached();
}

void
MGeneticAlgorithmEvaluator::Detach(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_evaler = 0x0;
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

const JEGA::Algorithms::GeneticAlgorithmEvaluator&
MGeneticAlgorithmEvaluator::Manifest(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_evaler != 0x0)
    return *this->_evaler;
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

MGeneticAlgorithmEvaluator::MGeneticAlgorithmEvaluator(
    const JEGA::Algorithms::GeneticAlgorithmEvaluator* toWrap
    ) :
        _evaler(toWrap)
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_evaler != 0x0)
}




/*
================================================================================
End Namespace
================================================================================
*/
        } // namespace Managed
    } // namespace FrontEnd
} // namespace JEGA

