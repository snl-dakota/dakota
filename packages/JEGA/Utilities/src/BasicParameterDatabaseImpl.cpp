/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class BasicParameterDatabaseImpl.

    NOTES:

        See notes of BasicParameterDatabaseImpl.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Fri Jan 06 07:35:56 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the BasicParameterDatabaseImpl class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <ostream>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/BasicParameterDatabaseImpl.hpp>





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
    namespace Utilities {




/*
================================================================================
Static Member Data Definitions
================================================================================
*/





/*
================================================================================
Private Template Method Implementations
================================================================================
*/
template <typename Map_T>
void
BasicParameterDatabaseImpl::DumpValueMap(
    const Map_T& theMap,
    const string& desc,
    ostream& stream
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGA_IF_THREADSAFE(typename Map_T::scoped_lock l(theMap);)
    PreDump(theMap, desc, stream);
    for(typename Map_T::const_iterator it(theMap.begin());
        it!=theMap.end(); ++it)
            stream << (*it).first << " = " << (*it).second << '\n';
    PostDump(theMap, stream);
}


template <typename CMap>
void
BasicParameterDatabaseImpl::DumpContainerMap(
    const CMap& theMap,
    const string& desc,
    ostream& stream
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGA_IF_THREADSAFE(typename CMap::scoped_lock l(theMap);)
    PreDump(theMap, desc, stream);
    for(typename CMap::const_iterator it(theMap.begin());
        it!=theMap.end(); ++it)
    {
        stream << (*it).first << " = \n";
        DumpContainer((*it).second, stream);
    }
    PostDump(theMap, stream);
}

template <typename CType>
void
BasicParameterDatabaseImpl::DumpContainer(
    const CType& theCont,
    ostream& stream
    )
{
    EDDY_FUNC_DEBUGSCOPE
    stream << "[";

    if(!theCont.empty())
    {
        typename CType::const_iterator e(theCont.end()); --e;
        for(typename CType::const_iterator jt(theCont.begin());
            jt!=e; ++jt) stream << (*jt) << ", ";
        stream << (*e);
    }
    stream << "]\n";
}

template <typename MMap, typename MType>
void
BasicParameterDatabaseImpl::DumpMatrixMap(
    const MMap& theMap,
    const string& desc,
    ostream& stream
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGA_IF_THREADSAFE(typename MMap::scoped_lock l(theMap);)
    PreDump(theMap, desc, stream);
    for(typename MMap::const_iterator it(theMap.begin());
        it!=theMap.end(); ++it)
    {
        stream << (*it).first << " = \n";
        for(typename MType::const_iterator jt((*it).second.begin());
            jt!=(*it).second.end(); ++jt)
                DumpContainer(*jt, stream);
    }
    PostDump(theMap, stream);
}

template <typename Map_T>
void
BasicParameterDatabaseImpl::PreDump(
    const Map_T& theMap,
    const string& desc,
    ostream& stream
    )
{
    EDDY_FUNC_DEBUGSCOPE
    if(!theMap.empty())
        stream << desc << ":\n---------------------------------------------\n";
}

template <typename Map_T>
void
BasicParameterDatabaseImpl::PostDump(
    const Map_T& theMap,
    ostream& stream
    )
{
    EDDY_FUNC_DEBUGSCOPE
    if(!theMap.empty()) stream << '\n';
}











/*
================================================================================
In Namesapce File Scope Utility Class Implementations
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
#define HAS_PARAM_METHOD(partype, mapname) \
    bool \
    BasicParameterDatabaseImpl::Has##partype##Param( \
        const string& tag \
        ) const \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return HasParam(tag, mapname); \
    }

HAS_PARAM_METHOD(Integral, _intParams)
HAS_PARAM_METHOD(Short, _shortParams)
HAS_PARAM_METHOD(Double, _doubleParams)
HAS_PARAM_METHOD(SizeType, _sizeTParams)
HAS_PARAM_METHOD(Boolean, _boolParams)
HAS_PARAM_METHOD(String, _stringParams)
HAS_PARAM_METHOD(DoubleVector, _doubleVectorParams)
HAS_PARAM_METHOD(IntVector, _intVectorParams)
HAS_PARAM_METHOD(DoubleMatrix, _doubleMatrixParams)
HAS_PARAM_METHOD(StringVector, _stringVectorParams)


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
#define GET_PARAM_METHOD(partype, cpptype, mapname) \
    cpptype \
    BasicParameterDatabaseImpl::Get##partype( \
        const string& tag \
        ) const \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return GetParamValue<cpptype>(tag, mapname); \
    }

GET_PARAM_METHOD(Integral, int, _intParams)
GET_PARAM_METHOD(Short, short, _shortParams)
GET_PARAM_METHOD(Double, double, _doubleParams)
GET_PARAM_METHOD(SizeType, size_t, _sizeTParams)
GET_PARAM_METHOD(Boolean, bool, _boolParams)
GET_PARAM_METHOD(String, string, _stringParams)
GET_PARAM_METHOD(DoubleVector, JEGA::DoubleVector, _doubleVectorParams)
GET_PARAM_METHOD(IntVector, JEGA::IntVector, _intVectorParams)
GET_PARAM_METHOD(DoubleMatrix, JEGA::DoubleMatrix, _doubleMatrixParams)
GET_PARAM_METHOD(StringVector, JEGA::StringVector, _stringVectorParams)


#define ADD_PARAM_METHOD(partype, cpptype, mapname) \
    bool \
    BasicParameterDatabaseImpl::Add##partype##Param( \
        const string& tag, \
        const cpptype& value \
        ) \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return AddParamValue(tag, value, mapname); \
    }


ADD_PARAM_METHOD(Integral, int, _intParams)
ADD_PARAM_METHOD(Short, short, _shortParams)
ADD_PARAM_METHOD(Double, double, _doubleParams)
ADD_PARAM_METHOD(SizeType, size_t, _sizeTParams)
ADD_PARAM_METHOD(Boolean, bool, _boolParams)
ADD_PARAM_METHOD(String, string, _stringParams)
ADD_PARAM_METHOD(DoubleVector, JEGA::DoubleVector, _doubleVectorParams)
ADD_PARAM_METHOD(IntVector, JEGA::IntVector, _intVectorParams)
ADD_PARAM_METHOD(DoubleMatrix, JEGA::DoubleMatrix, _doubleMatrixParams)
ADD_PARAM_METHOD(StringVector, JEGA::StringVector, _stringVectorParams)

string
BasicParameterDatabaseImpl::Dump(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    ostringstream ostr;
    Dump(ostr);
    return ostr.str();
}

void
BasicParameterDatabaseImpl::Dump(
    ostream& stream
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    DumpValueMap(_intParams, "Integer Parameters", stream);
    DumpValueMap(_shortParams, "Short Parameters", stream);
    DumpValueMap(_doubleParams, "Double Parameters", stream);
    DumpValueMap(_sizeTParams, "Size Type Parameters", stream);
    DumpValueMap(_boolParams, "Boolean Parameters", stream);
    DumpValueMap(_stringParams, "String Parameters", stream);
    DumpContainerMap(_doubleVectorParams, "Double Vector Parameters", stream);
    DumpContainerMap(_intVectorParams, "Integer  Vector Parameters", stream);
    DumpMatrixMap<DoubleMatrixParamMap, JEGA::DoubleMatrix >(
        _doubleMatrixParams, "Double Matrix Parameters", stream
        );
    DumpContainerMap(_stringVectorParams, "String Vector Parameters", stream);

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

BasicParameterDatabaseImpl::BasicParameterDatabaseImpl(
    ) :
        _intParams(),
        _doubleParams(),
        _sizeTParams(),
        _boolParams(),
        _stringParams(),
        _doubleVectorParams(),
        _intVectorParams(),
        _doubleMatrixParams(),
        _stringVectorParams()
{
}

BasicParameterDatabaseImpl::~BasicParameterDatabaseImpl(
    )
{
}





/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA
