/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Managed Front End

    CONTENTS:

        Implementation of class MFitnessRecord.

    NOTES:

        See notes of MFitnessRecord.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories, Albuquerque NM

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.1.0

    CHANGES:

        Fri Sep 28 09:07:37 2007 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the MFitnessRecord class.
 */




/*
================================================================================
Includes
================================================================================
*/
#include <stdafx.h>
#include <MDesign.hpp>
#include <MFitnessRecord.hpp>

#pragma unmanaged
#include <../include/FitnessRecord.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#pragma managed







/*
================================================================================
Namespace Using Directives
================================================================================
*/
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

JEGA::Algorithms::FitnessRecord&
MFitnessRecord::Manifest(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return *this->_guts;
}


bool
MFitnessRecord::AddFitness(
    MDesign MOH des,
    double fitness
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return _guts->AddFitness(&des->Manifest(), fitness);
}

double
MFitnessRecord::GetAverageFitness(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return _guts->GetAverageFitness();
}

double
MFitnessRecord::GetFitness(
    MDesign MOH des
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return _guts->GetFitness(des->Manifest());
}

double
MFitnessRecord::GetMaxFitness(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return _guts->GetMaxFitness();
}

double
MFitnessRecord::GetMinFitness(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return _guts->GetMinFitness();
}

double
MFitnessRecord::GetTotalFitness(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return _guts->GetTotalFitness();
}

std::size_t
MFitnessRecord::GetSize(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return _guts->GetSize();
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
MFitnessRecord::MANAGED_DISPOSE(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    delete _guts;
    _guts = 0x0;
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

MFitnessRecord::MFitnessRecord(
    ) :
        _guts(new FitnessRecord())
{
    EDDY_FUNC_DEBUGSCOPE
}

MFitnessRecord::MFitnessRecord(
    size_t initSize
    ) :
        _guts(new FitnessRecord(initSize))
{
    EDDY_FUNC_DEBUGSCOPE
}

MFitnessRecord::~MFitnessRecord(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    MANAGED_DISPOSE();
}








/*
================================================================================
End Namespace
================================================================================
*/
        } // namespace Managed
    } // namespace FrontEnd
} // namespace JEGA

