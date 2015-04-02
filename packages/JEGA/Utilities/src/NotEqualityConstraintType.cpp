/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class NotEqualityConstraintType.

    NOTES:

        See notes of NotEqualityConstraintType.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Tue Aug 29 11:01:17 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the NotEqualityConstraintType class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utilities/include/Math.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/NotEqualityConstraintType.hpp>

#ifdef JEGA_HAVE_BOOST
#include <boost/lexical_cast.hpp>
#else
#include <sstream>
#endif





/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace eddy::utilities;








/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Utilities {



template <typename T>
std::string
AsString(
    const T& val
    )
{
    EDDY_FUNC_DEBUGSCOPE

#ifdef JEGA_HAVE_BOOST
	return boost::lexical_cast<std::string>(val);
#else
    ostringstream ostr;
    ostr << val;
    return ostr.str();
#endif
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
void
NotEqualityConstraintType::SetTabooValue(
    double val
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_value = val;
    this->_viol =
        (this->_value == 0.0) ? 10.0 :
        Math::Ceil(Math::Abs(this->_value) / 10.0) * 10;
}







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

string
NotEqualityConstraintType::ToString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return string("Not-Equality");
}

bool
NotEqualityConstraintType::IsNatureLocked(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return false;
}

double
NotEqualityConstraintType::GetViolationAmount(
    double of
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // if "of" is equal to "_value", it violates by some nominal amount.
    // We will have the amount be on the order of magnitude of the value.
    return (of != this->_value) ? 0.0 : this->_viol;
}

ConstraintTypeBase*
NotEqualityConstraintType::Clone(
    ConstraintInfo& forCNI
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new NotEqualityConstraintType(*this, forCNI);
}

std::string
NotEqualityConstraintType::GetEquation(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return "n(x)!=" + AsString(this->_value) + " +/-" + AsString(this->_viol);
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


NotEqualityConstraintType::NotEqualityConstraintType(
    ConstraintInfo& info
    ) :
        ConstraintTypeBase(info),
        _value(0.0),
        _viol(10.0)
{
    EDDY_FUNC_DEBUGSCOPE
}

NotEqualityConstraintType::NotEqualityConstraintType(
    const NotEqualityConstraintType& copy,
    ConstraintInfo& info
    ) :
        ConstraintTypeBase(copy, info),
        _value(copy._value),
        _viol(copy._viol)
{
    EDDY_FUNC_DEBUGSCOPE
}








/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA

