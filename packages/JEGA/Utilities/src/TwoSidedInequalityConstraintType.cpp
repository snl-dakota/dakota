/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class TwoSidedInequalityConstraintType.

    NOTES:

        See notes of TwoSidedInequalityConstraintType.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jun 10 15:04:50 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the TwoSidedInequalityConstraintType
 *        class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <cfloat>
#include <utilities/include/Math.hpp>
#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/TwoSidedInequalityConstraintType.hpp>

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
using namespace JEGA::Logging;







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
TwoSidedInequalityConstraintType::SetLowerValue(
    double val
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(val <= GetUpperValue());

    if(val > GetUpperValue())
    {
        JEGALOG_II_G(lquiet(), this,
            ostream_entry(lquiet(), "2-Sided Ineq. Con: Lower value cannot "
                "be larger than upper value.  Adjusting lower value of ")
                << val << " to match upper value of " << GetUpperValue()
            )
        val = GetUpperValue();
    }
    this->_lowerValue = val;
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
TwoSidedInequalityConstraintType::ToString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return string("Two Sided Inequality");
}

double
TwoSidedInequalityConstraintType::GetViolationAmount(
    double of
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_lowerValue <= GetUpperValue());

    const double viol = this->InequalityConstraintType::GetViolationAmount(of);
    if(viol != 0.0) return viol;
    return (of >= this->_lowerValue) ? 0.0 : (of - this->_lowerValue);
}

ConstraintTypeBase*
TwoSidedInequalityConstraintType::Clone(
    ConstraintInfo& forCNI
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new TwoSidedInequalityConstraintType(*this, forCNI);
}

std::string
TwoSidedInequalityConstraintType::GetEquation(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return AsString(this->_lowerValue) + "<=" + this->InequalityConstraintType::GetEquation();
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

TwoSidedInequalityConstraintType::TwoSidedInequalityConstraintType(
    ConstraintInfo& info
    ) :
        InequalityConstraintType(info),
        _lowerValue(-DBL_MAX)
{
    EDDY_FUNC_DEBUGSCOPE
    SetUpperValue(DBL_MAX);
}

TwoSidedInequalityConstraintType::TwoSidedInequalityConstraintType(
    const TwoSidedInequalityConstraintType& copy,
    ConstraintInfo& info
    ) :
        InequalityConstraintType(copy, info),
        _lowerValue(-DBL_MAX)
{
    EDDY_FUNC_DEBUGSCOPE
    SetUpperValue(DBL_MAX);
}







/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA
