/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Managed Front End

    CONTENTS:

        Implementation of class MSolution.

    NOTES:

        See notes of MSolution.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Feb 13 13:14:41 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the MSolution class.
 */




/*
================================================================================
Includes
================================================================================
*/
#include <stdafx.h>
#include <MDesign.hpp>
#include <MSolution.hpp>

#pragma unmanaged
#include <../Utilities/include/Design.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#pragma managed







/*
================================================================================
Namespace Using Directives
================================================================================
*/
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

double
MSolution::GetVariable(
    eddy::utilities::uint64_t num
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return Convert::ToDouble(MANAGED_LIST_ITEM(
		this->_X, static_cast<int>(num)
		));
}

double
MSolution::GetObjective(
    eddy::utilities::uint64_t num
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return Convert::ToDouble(MANAGED_LIST_ITEM(
		this->_F, static_cast<int>(num)
		));
}

double
MSolution::GetConstraint(
    eddy::utilities::uint64_t num
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return Convert::ToDouble(MANAGED_LIST_ITEM(
        this->_G, static_cast<int>(num)
		));
}

eddy::utilities::uint64_t
MSolution::GetID(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_id;
}

Object^
MSolution::GetTag(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_tag;
}


/*
================================================================================
Public Methods
================================================================================
*/


eddy::utilities::uint64_t
MSolution::GetNDV(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_X->Count;

} // MSolution::GetNDV

eddy::utilities::uint64_t
MSolution::GetNOF(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_F->Count;

} // MSolution::GetNOF

eddy::utilities::uint64_t
MSolution::GetNCN(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_G->Count;

} // MSolution::GetNCN

bool
MSolution::IsEvaluated(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->HasAttribute(Design::Evaluated);
}

bool
MSolution::IsFeasible(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->IsEvaluated() && !this->IsIllconditioned() &&
           this->HasAttribute(Design::FeasibleConstraints) &&
           this->HasAttribute(Design::FeasibleBounds);
}

bool
MSolution::SatisfiesConstraints(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->IsEvaluated() && !this->IsIllconditioned() &&
           this->HasAttribute(Design::FeasibleConstraints);
}

bool
MSolution::SatisfiesBounds(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->IsEvaluated() && !this->IsIllconditioned() &&
           this->HasAttribute(Design::FeasibleBounds);
}

bool
MSolution::IsIllconditioned(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->HasAttribute(Design::Illconditioned);
}





/*
================================================================================
Subclass Visible Methods
================================================================================
*/


bool
MSolution::HasAttribute(
    eddy::utilities::uint64_t attrIndex
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return (this->_attributes & (1 << attrIndex)) != 0;
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








/*
================================================================================
Structors
================================================================================
*/


MSolution::MSolution(
    const JEGA::Utilities::Design& des
    ) :
        _X(MANAGED_GCNEW DoubleVector(static_cast<int>(des.GetNDV()))),
        _F(MANAGED_GCNEW DoubleVector(static_cast<int>(des.GetNOF()))),
        _G(MANAGED_GCNEW DoubleVector(static_cast<int>(des.GetNCN()))),
        _attributes(static_cast<eddy::utilities::bitmask8_t>(
            des.GetRawAttributes().to_ulong()
            )),
        _id(des.GetID()),
        _tag(MANAGED_NULL_HANDLE)
{
    EDDY_FUNC_DEBUGSCOPE

    // record the design variables first.
    const std::size_t ndv = des.GetNDV();
    for(std::size_t i=0; i<ndv; ++i)
        this->_X->Add(MANAGED_BOX(des.GetVariableValue(i)));

    // now the objective function values.
    const std::size_t nof = des.GetNOF();
    for(std::size_t i=0; i<nof; ++i)
        this->_F->Add(MANAGED_BOX(des.GetObjective(i)));

    // and now the constraint values.
    const std::size_t ncn = des.GetNCN();
    for(std::size_t i=0; i<ncn; ++i)
        this->_G->Add(MANAGED_BOX(des.GetConstraint(i)));

    void* tag = des.GetTag();
    if(tag != 0x0) this->_tag = GCHandle::FromIntPtr(IntPtr(tag)).Target;
}


MSolution::MSolution(
    MDesign MOH mDes
    ) :
        _X(MANAGED_GCNEW DoubleVector(static_cast<int>(mDes->GetNDV()))),
        _F(MANAGED_GCNEW DoubleVector(static_cast<int>(mDes->GetNOF()))),
        _G(MANAGED_GCNEW DoubleVector(static_cast<int>(mDes->GetNCN()))),
        _attributes(static_cast<eddy::utilities::bitmask8_t>(
            mDes->Manifest().GetRawAttributes().to_ulong()
            )),
        _id(static_cast<size_t>(mDes->GetID())),
        _tag(MANAGED_NULL_HANDLE)
{
    EDDY_FUNC_DEBUGSCOPE

    const JEGA::Utilities::Design& des = mDes->Manifest();

    // record the design variables first.
    const std::size_t ndv = des.GetNDV();
    for(std::size_t i=0; i<ndv; ++i)
        this->_X->Add(MANAGED_BOX(des.GetVariableValue(i)));

    // now the objective function values.
    const std::size_t nof = des.GetNOF();
    for(std::size_t i=0; i<nof; ++i)
        this->_F->Add(MANAGED_BOX(des.GetObjective(i)));

    // and now the constraint values.
    const std::size_t ncn = des.GetNCN();
    for(std::size_t i=0; i<ncn; ++i)
        this->_G->Add(MANAGED_BOX(des.GetConstraint(i)));

    void* tag = des.GetTag();
    if(tag != 0x0) this->_tag = GCHandle::FromIntPtr(IntPtr(tag)).Target;
}




/*
================================================================================
End Namespace
================================================================================
*/
        } // namespace Managed
    } // namespace FrontEnd
} // namespace JEGA

