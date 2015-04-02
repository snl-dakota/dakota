/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implemntation of class EqualityConstraintType.

    NOTES:

        See notes of EqualityConstraintType .hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jun 10 08:43:33 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the EqualityConstraintType class.
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
#include <../Utilities/include/EqualityConstraintType.hpp>

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
EqualityConstraintType::ToString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return string("Equality");
}

bool
EqualityConstraintType::IsNatureLocked(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return false;
}

double
EqualityConstraintType::GetViolationAmount(
    double of
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // if "of" is within "_viol" of "_value", it is ok
    const double diff = of - this->_value;
    return (Math::Abs(diff) <= Math::Abs(this->_viol)) ? 0.0 : diff;
}

ConstraintTypeBase*
EqualityConstraintType::Clone(
    ConstraintInfo& forCNI
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new EqualityConstraintType(*this, forCNI);
}

std::string
EqualityConstraintType::GetEquation(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return "h(x)=" + AsString(this->_value) + " +/- " + AsString(this->_viol);
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

EqualityConstraintType::EqualityConstraintType(
    ConstraintInfo& info
    ) :
        ConstraintTypeBase(info),
        _value(0.0),
        _viol(0.0)
{
    EDDY_FUNC_DEBUGSCOPE
}

EqualityConstraintType::EqualityConstraintType(
    const EqualityConstraintType& copy,
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
