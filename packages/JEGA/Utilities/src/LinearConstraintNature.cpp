/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementaion of class LinearConstraintNature.

    NOTES:

        See notes of LinearConstraintNature.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jun 10 10:23:50 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the LinearConstraintNature class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/Design.hpp>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignTarget.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ConstraintInfo.hpp>
#include <../Utilities/include/LinearConstraintNature.hpp>



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
Static Member Data Definitions
================================================================================
*/








/*
================================================================================
Mutators
================================================================================
*/
void
LinearConstraintNature::SetCoefficients(
    const DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(
        this->GetType().GetConstraintInfo().GetDesignTarget().GetNDV() ==
        coeffs.size()
        );

#ifdef JEGA_LOGGING_ON

    std::size_t reqNumCoeffs =
        this->GetType().GetConstraintInfo().GetDesignTarget().GetNDV();

    if((reqNumCoeffs > 0) && (reqNumCoeffs != coeffs.size()))
    {
        const string label(this->GetType().GetConstraintInfo().GetLabel());

        JEGAIFLOG_CF_II_G(reqNumCoeffs < coeffs.size(), lquiet(), this,
            ostream_entry(
                    lquiet(), "Linear Constraint (" + label + "): Received "
                    )
                << coeffs.size() << " coefficients.  Expected " << reqNumCoeffs
                << ".  This constraint cannot be automatically evaluated."
            )

        JEGAIFLOG_CF_II_G(reqNumCoeffs > coeffs.size(), lquiet(), this,
            ostream_entry(
                    lquiet(), "Linear Constraint (" + label + "): Received "
                    )
                << coeffs.size() << " coefficients.  Expected " << reqNumCoeffs
                << ".  Extra coefficients will be ignored."
            )
    }

#endif

    this->_coeffs = coeffs;
}


void
LinearConstraintNature::SetCoefficient(
    DoubleVector::size_type elem,
    double val
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(elem < this->_coeffs.size());
    this->_coeffs[elem] = val;
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

const string&
LinearConstraintNature::GetName(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string name("Linear");
    return name;

} // LinearConstraintNature::GetName

/*
================================================================================
Subclass Visible Methods
================================================================================
*/


string
LinearConstraintNature::ToString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return GetName();
}

ConstraintNatureBase*
LinearConstraintNature::Clone(
    ConstraintTypeBase& forType
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new LinearConstraintNature(*this, forType);
}


bool
LinearConstraintNature::EvaluateConstraint(
    Design& des
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // Store the associated constraint info object for repeated use
    const ConstraintInfo& cnInfo = this->GetType().GetConstraintInfo();

    // Make sure that the number of coefficients is correct (debug only).
    EDDY_ASSERT(cnInfo.GetDesignTarget().GetNDV() == this->_coeffs.size());

    // Store the design target for repeated use
    const DesignTarget& target = cnInfo.GetDesignTarget();

    // store the number of dv's for repeated use.
    const std::size_t ndv = target.GetNDV();

    // if the number of coefficients is not equal to the number of
    // design variables, we have a problem and cannot complete this
    // calculation.
    if(ndv != this->_coeffs.size()) return false;

    // prepare to calculate the result.  To do so, we will need to
    // iterate the info objects.
    double result = 0.0;

    for(size_t i=0; i<ndv; ++i)
        result += this->_coeffs[i] * des.GetVariableValue(i);

    // now record the result with the Design class object.
    des.SetConstraint(cnInfo.GetNumber(), result);
    return true;
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


LinearConstraintNature::LinearConstraintNature(
    ConstraintTypeBase& type
    ) :
        ConstraintNatureBase(type)
{
    EDDY_FUNC_DEBUGSCOPE
}

LinearConstraintNature::LinearConstraintNature(
    const LinearConstraintNature& copy,
    ConstraintTypeBase& type
    ) :
        ConstraintNatureBase(copy, type)
{
    EDDY_FUNC_DEBUGSCOPE
}

LinearConstraintNature::~LinearConstraintNature(
    )
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
