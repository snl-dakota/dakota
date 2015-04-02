/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class ParameterExtractor.

    NOTES:

        See notes of ParameterExtractor.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Tue Feb 28 09:53:33 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the ParameterExtractor class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ParameterDatabase.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>







/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA;
using namespace JEGA::Logging;







/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Utilities {


/*
================================================================================
Private Template Method Definitions
================================================================================
*/

template<typename R>
R
ParameterExtractor::_GetParamFromDB(
    const JEGA::Utilities::ParameterDatabase& db,
    const std::string& tag,
    R (JEGA::Utilities::ParameterDatabase::*func)(const std::string&) const
    )
{
    EDDY_FUNC_DEBUGSCOPE

    try
    {
        return (db.*func)(tag);
    }
    catch(const exception& JEGA_LOGGING_IF_ON(e))
    {
        JEGALOG_G(lquiet(),
            text_entry(lquiet(), "Error encountered while "
                "retrieving parameter named \"" + tag + "\" of type \"")
                << typeid(R).name() << "\" from parameter database reading: "
                << e.what()
            )
        throw;
    }
}

template<typename R>
bool
ParameterExtractor::_GetParamFromDB(
    const ParameterDatabase& db,
    const string& tag,
    R& into,
    R (ParameterDatabase::*func)(const string&) const
    )
{
    EDDY_FUNC_DEBUGSCOPE

    try
    {
        into = _GetParamFromDB<R>(db, tag, func);
        return true;
    }
    catch(const exception&)
    {
        return false;
    }
}




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
#define GET_VAL_FROM_DB_METHOD_I(partype, cpptype) \
    cpptype \
    ParameterExtractor::Get##partype##FromDB( \
        const JEGA::Utilities::ParameterDatabase& db, \
        const std::string& tag \
        ) \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return _GetParamFromDB<cpptype>( \
            db, tag, &ParameterDatabase::Get##partype \
            ); \
    }

GET_VAL_FROM_DB_METHOD_I(Integral, int)
GET_VAL_FROM_DB_METHOD_I(Double, double)
GET_VAL_FROM_DB_METHOD_I(SizeType, size_t)
GET_VAL_FROM_DB_METHOD_I(Boolean, bool)
GET_VAL_FROM_DB_METHOD_I(String, string)
GET_VAL_FROM_DB_METHOD_I(DoubleVector, JEGA::DoubleVector)
GET_VAL_FROM_DB_METHOD_I(IntVector, JEGA::IntVector)
GET_VAL_FROM_DB_METHOD_I(DoubleMatrix, JEGA::DoubleMatrix)
GET_VAL_FROM_DB_METHOD_I(StringVector, JEGA::StringVector)


#define GET_VAL_FROM_DB_METHOD_B(partype, cpptype) \
    bool \
    ParameterExtractor::Get##partype##FromDB( \
        const ParameterDatabase& db, \
        const string& tag, \
        cpptype& into \
        ) \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return _GetParamFromDB<cpptype>( \
            db, tag, into, &ParameterDatabase::Get##partype \
            ); \
    }

GET_VAL_FROM_DB_METHOD_B(Integral, int)
GET_VAL_FROM_DB_METHOD_B(Double, double)
GET_VAL_FROM_DB_METHOD_B(SizeType, size_t)
GET_VAL_FROM_DB_METHOD_B(Boolean, bool)
GET_VAL_FROM_DB_METHOD_B(String, string)
GET_VAL_FROM_DB_METHOD_B(DoubleVector, JEGA::DoubleVector)
GET_VAL_FROM_DB_METHOD_B(IntVector, JEGA::IntVector)
GET_VAL_FROM_DB_METHOD_B(DoubleMatrix, JEGA::DoubleMatrix)
GET_VAL_FROM_DB_METHOD_B(StringVector, JEGA::StringVector)



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








/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA
