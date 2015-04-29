/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Managed Front End

    CONTENTS:

        Implementation of class MDesign.

    NOTES:

        See notes of MDesign.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Feb 15 08:41:41 2007 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the MDesign class.
 */




/*
================================================================================
Includes
================================================================================
*/
#include <stdafx.h>
#include <MDesign.hpp>

#pragma unmanaged
#include <../Utilities/include/Design.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#pragma managed






/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace System;
using namespace JEGA::Utilities;
using namespace System::Runtime::InteropServices;







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


GCHandle
GetGCHandle(
    JEGA::Utilities::Design* des
    )
{
    void* tag = des->GetTag();
    if(tag == 0x0) return GCHandle();
    IntPtr ip(tag);
    try { return GCHandle::FromIntPtr(ip); } catch (...) { return GCHandle(); }
}

void
FreeGCHandle(
    JEGA::Utilities::Design* des
    )
{
    GCHandle gch = GetGCHandle(des);
    if(gch.IsAllocated::get()) gch.Free();
}








/*
================================================================================
Mutators
================================================================================
*/

void
MDesign::SetVariable(
    eddy::utilities::uint64_t num,
    double val
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_design != 0x0)
    this->_design->SetVariableValue(static_cast<size_t>(num), val);
}

void
MDesign::SetObjective(
    eddy::utilities::uint64_t num,
    double val
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_design != 0x0)
    this->_design->SetObjective(static_cast<size_t>(num), val);
}

void
MDesign::SetConstraint(
    eddy::utilities::uint64_t num,
    double val
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_design != 0x0)
    this->_design->SetConstraint(static_cast<size_t>(num), val);
}

void
MDesign::SetTag(
    Object MOH tag
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_design != 0x0)
    if(this->_design->GetTag() != 0x0) FreeGCHandle(this->_design);
    GCHandle gch = GCHandle::Alloc(tag);
    this->_design->SetTag(GCHandle::ToIntPtr(gch).ToPointer());
}


/*
================================================================================
Accessors
================================================================================
*/

double
MDesign::GetVariable(
    eddy::utilities::uint64_t num
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_design != 0x0)
    return this->_design->GetVariableValue(static_cast<size_t>(num));
}

double
MDesign::GetObjective(
    eddy::utilities::uint64_t num
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_design != 0x0)
    return this->_design->GetObjective(static_cast<size_t>(num));
}

double
MDesign::GetConstraint(
    eddy::utilities::uint64_t num
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_design != 0x0)
    return this->_design->GetConstraint(static_cast<size_t>(num));
}

eddy::utilities::uint64_t
MDesign::GetID(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_design != 0x0)
    return this->_design->GetID();
}

Object^
MDesign::GetTag(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_design != 0x0)
    GCHandle gch = GetGCHandle(this->_design);
    return gch.IsAllocated::get() ? gch.Target::get() : MANAGED_NULL_HANDLE;
}

/*
================================================================================
Public Methods
================================================================================
*/
eddy::utilities::uint64_t
MDesign::GetNDV(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_design != 0x0)
    return this->_design->GetNDV();

} // MDesign::GetNDV

eddy::utilities::uint64_t
MDesign::GetNOF(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_design != 0x0)
    return this->_design->GetNOF();

} // MDesign::GetNOF

eddy::utilities::uint64_t
MDesign::GetNCN(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_design != 0x0)
    return this->_design->GetNCN();

} // MDesign::GetNCN

bool
MDesign::IsEvaluated(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_design != 0x0)
    return this->_design->IsEvaluated();
}

bool
MDesign::IsFeasible(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_design != 0x0)
    return this->_design->IsFeasible();
}

bool
MDesign::SatisfiesBounds(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_design != 0x0)
    return this->_design->SatisfiesBounds();
}

bool
MDesign::SatisfiesConstraints(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_design != 0x0)
    return this->_design->SatisfiesConstraints();
}

void
MDesign::SetIllconditioned(
    bool lval
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_design != 0x0)
    return this->_design->SetIllconditioned(lval);
}

bool
MDesign::IsIllconditioned(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_design != 0x0)
    return this->_design->IsIllconditioned();
}

void
MDesign::ResetDesign(
    Design* toWrap
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_design = toWrap;
    EDDY_ASSERT(this->_design != 0x0)
}

MDesign MOH
MDesign::Create(
    MDesign MOH hint
    )
{
    EDDY_FUNC_DEBUGSCOPE
    const DesignTarget& target = hint->Manifest().GetDesignTarget();
    Design* des = target.GetNewDesign();
    MDesign MOH ret = MANAGED_GCNEW MDesign(des);
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

JEGA::Utilities::Design&
MDesign::Manifest(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_design != 0x0)
    return *this->_design;
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
static
MDesign::MDesign(
    )
{
    Design::SetDisposeCallback(&FreeGCHandle);
}

MDesign::MDesign(
    JEGA::Utilities::Design* toWrap
    ) :
        _design(toWrap)
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_design != 0x0)
}

MDesign::~MDesign(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_design = 0x0;
}






/*
================================================================================
End Namespace
================================================================================
*/
        } // namespace Managed
    } // namespace FrontEnd
} // namespace JEGA

