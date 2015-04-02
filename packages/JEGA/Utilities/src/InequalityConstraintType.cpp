/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class InequalityConstraintType.

    NOTES:

        See notes of InequalityConstraintType.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jun 10 08:43:25 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the InequalityConstraintType class.
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
#include <../Utilities/include/InequalityConstraintType.hpp>

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
InequalityConstraintType::ToString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return string("Inequality");
}

bool
InequalityConstraintType::IsNatureLocked(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return false;
}

double
InequalityConstraintType::GetViolationAmount(
    double of
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // if "of" is less than or equal to "_upperValue", it is ok.
    return (of <= this->_upperValue) ? 0.0 : (of - this->_upperValue);
}

ConstraintTypeBase*
InequalityConstraintType::Clone(
    ConstraintInfo& forCNI
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new InequalityConstraintType(*this, forCNI);
}

std::string
InequalityConstraintType::GetEquation(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return "g(x)<=" + AsString(this->_upperValue);
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

InequalityConstraintType::InequalityConstraintType(
    ConstraintInfo& info
    ) :
        ConstraintTypeBase(info),
        _upperValue(0.0)
{
    EDDY_FUNC_DEBUGSCOPE
}

InequalityConstraintType::InequalityConstraintType(
    const InequalityConstraintType& copy,
    ConstraintInfo& info
    ) :
        ConstraintTypeBase(copy, info),
        _upperValue(copy._upperValue)
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
