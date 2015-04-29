/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Front End

    CONTENTS:

        Implementation of class ProblemConfig.

    NOTES:

        See notes of ProblemConfig.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Fri Jan 06 10:10:26 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the ProblemConfig class.
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
#include <../FrontEnd/Core/include/ConfigHelper.hpp>
#include <../FrontEnd/Core/include/ProblemConfig.hpp>




/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Logging;
using namespace JEGA::Utilities;







/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace FrontEnd {





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
void
ProblemConfig::SetDiscardTracking(
    bool track
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_theTarget.SetTrackDiscards(track);
}

bool
ProblemConfig::GetDiscardTracking(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_theTarget.GetTrackDiscards();
}

void
ProblemConfig::SetMaxGuffSize(
    std::size_t mgs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_theTarget.SetMaxGuffSize(mgs);
}

std::size_t
ProblemConfig::GetMaxGuffSize(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_theTarget.GetMaxGuffSize();
}

bool
ProblemConfig::AddContinuumRealVariable(
    const string& label,
    double lowerBound,
    double upperBound,
    int precision
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ConfigHelper::AddContinuumRealVariable(
        this->_theTarget, label, lowerBound, upperBound, precision
        );
}

bool
ProblemConfig::AddDiscreteRealVariable(
    const string& label,
    const JEGA::DoubleVector& values
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGAIFLOG_CF_G_F(values.empty(),
        text_entry(lfatal(),
            "An attempt was made to add a discrete real variable named \"" +
            label + "\" with no values."
            )
        )

    return ConfigHelper::AddDiscreteRealVariable(
        this->_theTarget, label, values
        );
}

bool
ProblemConfig::AddContinuumIntegerVariable(
    const string& label,
    int lowerBound,
    int upperBound
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ConfigHelper::AddContinuumIntegerVariable(
        this->_theTarget, label, lowerBound, upperBound
        );
}

bool
ProblemConfig::AddDiscreteIntegerVariable(
    const string& label,
    const JEGA::IntVector& values
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGAIFLOG_CF_G_F(values.empty(),
        text_entry(lfatal(),
            "An attempt was made to add a discrete integer variable named \"" +
            label + "\" with no values."
            )
        )

    return ConfigHelper::AddDiscreteIntegerVariable(
        this->_theTarget, label, values
        );
}

bool
ProblemConfig::AddBooleanVariable(
    const std::string& label
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ConfigHelper::AddBooleanVariable(this->_theTarget, label);
}

bool
ProblemConfig::AddLinearMinimizeObjective(
    const string& label,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ConfigHelper::AddLinearMinimizeObjective(
        this->_theTarget, label, coeffs
        );
}

bool
ProblemConfig::AddLinearMaximizeObjective(
    const string& label,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ConfigHelper::AddLinearMaximizeObjective(
        this->_theTarget, label, coeffs
        );
}

bool
ProblemConfig::AddLinearSeekValueObjective(
    const string& label,
    double value,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ConfigHelper::AddLinearSeekValueObjective(
        this->_theTarget, label, value, coeffs
        );
}

bool
ProblemConfig::AddLinearSeekRangeObjective(
    const string& label,
    double lowerBound,
    double upperBound,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ConfigHelper::AddLinearSeekRangeObjective(
        this->_theTarget, label, lowerBound, upperBound, coeffs
        );
}

bool
ProblemConfig::AddNonlinearMinimizeObjective(
    const string& label
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ConfigHelper::AddNonlinearMinimizeObjective(
        this->_theTarget, label
        );
}

bool
ProblemConfig::AddNonlinearMaximizeObjective(
    const string& label
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ConfigHelper::AddNonlinearMaximizeObjective(
        this->_theTarget, label
        );
}

bool
ProblemConfig::AddNonlinearSeekValueObjective(
    const string& label,
    double value
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ConfigHelper::AddNonlinearSeekValueObjective(
        this->_theTarget, label, value
        );
}

bool
ProblemConfig::AddNonlinearSeekRangeObjective(
    const string& label,
    double lowerBound,
    double upperBound
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ConfigHelper::AddNonlinearSeekRangeObjective(
        this->_theTarget, label, lowerBound, upperBound
        );
}

bool
ProblemConfig::AddLinearInequalityConstraint(
    const string& label,
    double upperLimit,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ConfigHelper::AddLinearInequalityConstraint(
        this->_theTarget, label, upperLimit, coeffs
        );
}

bool
ProblemConfig::AddLinearEqualityConstraint(
    const string& label,
    double target,
    double allowedViol,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ConfigHelper::AddLinearEqualityConstraint(
        this->_theTarget, label, target, allowedViol, coeffs
        );
}

bool
ProblemConfig::AddLinearNotEqualityConstraint(
    const string& label,
    double tabooValue,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ConfigHelper::AddLinearNotEqualityConstraint(
        this->_theTarget, label, tabooValue, coeffs
        );
}

bool
ProblemConfig::AddLinearTwoSidedInequalityConstraint(
    const string& label,
    double lowerLimit,
    double upperLimit,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ConfigHelper::AddLinearTwoSidedInequalityConstraint(
        this->_theTarget, label, lowerLimit, upperLimit, coeffs
        );
}

bool
ProblemConfig::AddNonlinearInequalityConstraint(
    const string& label,
    double upperLimit
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ConfigHelper::AddNonlinearInequalityConstraint(
        this->_theTarget, label, upperLimit
        );
}

bool
ProblemConfig::AddNonlinearEqualityConstraint(
    const string& label,
    double target,
    double allowedViol
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ConfigHelper::AddNonlinearEqualityConstraint(
        this->_theTarget, label, target, allowedViol
        );
}

bool
ProblemConfig::AddNonlinearNotEqualityConstraint(
    const string& label,
    double tabooValue
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ConfigHelper::AddNonlinearNotEqualityConstraint(
        this->_theTarget, label, tabooValue
        );
}

bool
ProblemConfig::AddNonlinearTwoSidedInequalityConstraint(
    const string& label,
    double lowerLimit,
    double upperLimit
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ConfigHelper::AddNonlinearTwoSidedInequalityConstraint(
        this->_theTarget, label, lowerLimit, upperLimit
        );
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

ProblemConfig::ProblemConfig(
    ) :
        _theTarget()
{
    EDDY_FUNC_DEBUGSCOPE
}



/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace FrontEnd
} // namespace JEGA

