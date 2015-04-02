/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Managed Front End

    CONTENTS:

        Implementation of class ManagedUtils.

    NOTES:

        See notes of ManagedUtils.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Feb 08 16:35:10 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the ManagedUtils class.
 */




/*
================================================================================
Includes
================================================================================
*/
#include <stdafx.h>
#include <ManagedUtils.hpp>

#pragma unmanaged
#include <utilities/include/EDDY_DebugScope.hpp>
#pragma managed




/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
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








/*
================================================================================
Public Methods
================================================================================
*/

System::String MOH
ManagedUtils::ToSysString(
    const char* cstr
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return MANAGED_GCNEW System::String(cstr);
}

System::String MOH
ManagedUtils::ToSysString(
    const string& stdStr
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ToSysString(stdStr.c_str());
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




JEGA::DoubleVector
ToStdDoubleVector(
    JEGA::FrontEnd::Managed::DoubleVector MOH ar
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGA::DoubleVector ret;

    MANAGED_IENUMERATOR(System::Double) MOH oe = ar->GetEnumerator();
    while(oe->MoveNext())
    {
        // This cannot be reserve and push_back because of problems
        // converting unboxed value types to native reference types.
        double tpb = System::Convert::ToDouble(oe->Current);
        ret.push_back(tpb);
    }

    return ret;
}

JEGA::FrontEnd::Managed::DoubleVector MOH
ToSysDoubleVector(
    const JEGA::DoubleVector& ar
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA::FrontEnd::Managed::DoubleVector MOH ret =
        MANAGED_GCNEW JEGA::FrontEnd::Managed::DoubleVector(
            static_cast<int>(ar.size())
            );

    for(JEGA::DoubleVector::const_iterator it(ar.begin()); it!=ar.end(); ++it)
        ret->Add(*it);

    return ret;
}

JEGA::IntVector
ToStdIntVector(
    JEGA::FrontEnd::Managed::IntVector MOH ar
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGA::IntVector ret;

    MANAGED_IENUMERATOR(System::Int32) MOH oe = ar->GetEnumerator();
    while(oe->MoveNext())
    {
        // This cannot be reserve and push_back because of problems
        // converting unboxed value types to native reference types.
        int tpb = System::Convert::ToInt32(oe->Current);
        ret.push_back(tpb);
    }

    return ret;
}

JEGA::FrontEnd::Managed::IntVector MOH
ToSysIntVector(
    const JEGA::IntVector& ar
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA::FrontEnd::Managed::IntVector MOH ret =
        MANAGED_GCNEW JEGA::FrontEnd::Managed::IntVector(
            static_cast<int>(ar.size())
            );

    for(JEGA::IntVector::const_iterator it(ar.begin()); it!=ar.end(); ++it)
        ret->Add(*it);

    return ret;
}

JEGA::StringVector
ToStdStringVector(
    JEGA::FrontEnd::Managed::StringVector MOH ar
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGA::StringVector ret;

    MANAGED_IENUMERATOR(System::String MOH) MOH oe = ar->GetEnumerator();
    while(oe->MoveNext())
    {
        // This cannot be reserve and push_back because of problems
        // converting unboxed value types to native reference types.
        std::string tpb = ToStdStr(oe->Current);
        ret.push_back(tpb);
    }

    return ret;
}

JEGA::FrontEnd::Managed::StringVector MOH
ToSysStringVector(
    const JEGA::StringVector& ar
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA::FrontEnd::Managed::StringVector MOH ret =
        MANAGED_GCNEW JEGA::FrontEnd::Managed::StringVector(
            static_cast<int>(ar.size())
            );

    for(JEGA::StringVector::const_iterator it(ar.begin());
        it!=ar.end(); ++it) ret->Add(ManagedUtils::ToSysString(*it));

    return ret;
}

std::string
ToStdStr(
    System::String MOH sysStr
    )
{
    EDDY_FUNC_DEBUGSCOPE
    const char* chrs =
        (const char*)(Marshal::StringToHGlobalAnsi(sysStr)).ToPointer();
    std::string ret(chrs);
    Marshal::FreeHGlobal(System::IntPtr((void*)chrs));
    return ret;
}


JEGA::DoubleMatrix
ToStdDoubleMatrix(
    JEGA::FrontEnd::Managed::DoubleMatrix MOH ar
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGA::DoubleMatrix ret;
    ret.resize(ar->Count);

    JEGA::DoubleMatrix::size_type i=0;

    MANAGED_IENUMERATOR(DoubleVector MOH) MOH oe = ar->GetEnumerator();

    while(oe->MoveNext())
        ret[i++] = ToStdDoubleVector(oe->Current);

    return ret;
}

JEGA::FrontEnd::Managed::DoubleMatrix MOH
ToSysDoubleMatrix(
    const JEGA::DoubleMatrix& ar
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA::FrontEnd::Managed::DoubleMatrix MOH ret =
        MANAGED_GCNEW JEGA::FrontEnd::Managed::DoubleMatrix(
            static_cast<int>(ar.size())
            );

    for(JEGA::DoubleMatrix::const_iterator it(ar.begin()); it!=ar.end(); ++it)
        ret->Add(ToSysDoubleVector(*it));

    return ret;
}

/*
================================================================================
End Namespace
================================================================================
*/
        } // namespace Managed
    } // namespace FrontEnd
} // namespace JEGA


