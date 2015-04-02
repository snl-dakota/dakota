/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class LinearObjectiveFunctionNature.

    NOTES:

        See notes of LinearObjectiveFunctionNature.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Sun Sep 14 16:40:20 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the LinearObjectiveFunctionNature class.
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
#include <../Utilities/include/ObjectiveFunctionInfo.hpp>
#include <../Utilities/include/ObjectiveFunctionTypeBase.hpp>
#include <../Utilities/include/LinearObjectiveFunctionNature.hpp>



/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA;
using namespace JEGA::Utilities;
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
LinearObjectiveFunctionNature::SetCoefficients(
    const DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE

#ifdef JEGA_LOGGING_ON

    std::size_t reqNumCoeffs =
        this->GetType().GetObjectiveFunctionInfo().GetDesignTarget().GetNDV();

    if((reqNumCoeffs > 0) && (reqNumCoeffs != coeffs.size()))
    {
        const string& label =
            this->GetType().GetObjectiveFunctionInfo().GetLabel();

        JEGAIFLOG_CF_II_G(reqNumCoeffs < coeffs.size(), lquiet(), this,
            ostream_entry(
                lquiet(), "Linear Objective (" + label + "): Received "
                )
                << coeffs.size() << " coefficients.  Expected " << reqNumCoeffs
                << ".  This objective cannot be automatically evaluated."
            )

        JEGAIFLOG_CF_II_G(reqNumCoeffs > coeffs.size(), lquiet(), this,
            ostream_entry(
                lquiet(), "Linear Objective (" + label + "): Received "
                )
                << coeffs.size() << " coefficients.  Expected " << reqNumCoeffs
                << ".  Extra coefficients will be ignored."
            )
    }

#endif

    this->_coeffs = coeffs;

} // LinearObjectiveFunctionNature::SetCoefficients

void
LinearObjectiveFunctionNature::SetCoefficient(
    DoubleVector::size_type elem,
    double val
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(elem < this->_coeffs.size());
    this->_coeffs[elem] = val;

} // LinearObjectiveFunctionNature::SetCoefficient






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
LinearObjectiveFunctionNature::GetName(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string name("Linear");
    return name;

} // LinearObjectiveFunctionNature::GetName





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
LinearObjectiveFunctionNature::ToString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return GetName();

} // LinearObjectiveFunctionNature::ToString

ObjectiveFunctionNatureBase*
LinearObjectiveFunctionNature::Clone(
    ObjectiveFunctionTypeBase& forCNI
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new LinearObjectiveFunctionNature(*this, forCNI);

} // LinearObjectiveFunctionNature::Clone

bool
LinearObjectiveFunctionNature::EvaluateObjective(
    Design& des
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // Get the number of design variables.
    const std::size_t ndv =
        this->GetType().GetObjectiveFunctionInfo().GetDesignTarget().GetNDV();

    // If we don't have enough coefficients, we cannot continue.
    if(ndv != this->_coeffs.size()) return false;

    double result = 0.0;
    for(size_t i=0; i<ndv; ++i)
        result += this->_coeffs[i] * des.GetVariableRep(i);

    des.SetObjective(
        this->GetType().GetObjectiveFunctionInfo().GetNumber(), result
        );
    return true;

} // LinearObjectiveFunctionNature::EvaluateObjective





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


LinearObjectiveFunctionNature::LinearObjectiveFunctionNature(
    ObjectiveFunctionTypeBase& type
    ) :
        ObjectiveFunctionNatureBase(type),
        _coeffs()
{
    EDDY_FUNC_DEBUGSCOPE

} // LinearObjectiveFunctionNature::LinearObjectiveFunctionNature

LinearObjectiveFunctionNature::LinearObjectiveFunctionNature(
    const LinearObjectiveFunctionNature& copy,
    ObjectiveFunctionTypeBase& type
    ) :
        ObjectiveFunctionNatureBase(copy, type),
        _coeffs(copy._coeffs)
{
    EDDY_FUNC_DEBUGSCOPE

} // LinearObjectiveFunctionNature::LinearObjectiveFunctionNature

LinearObjectiveFunctionNature::~LinearObjectiveFunctionNature(
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

