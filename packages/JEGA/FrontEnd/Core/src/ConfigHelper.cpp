/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Front End

    CONTENTS:

        Implementation of class ConfigHelper.

    NOTES:

        See notes of ConfigHelper.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Fri Feb 17 14:41:32 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the ConfigHelper class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../FrontEnd/Core/include/ConfigHelper.hpp>

#include <utilities/include/EDDY_DebugScope.hpp>

#include <../Utilities/include/DesignTarget.hpp>

// Design Variable Info Relavant Includes
#include <../Utilities/include/DesignVariableInfo.hpp>
#include <../Utilities/include/RealDesignVariableType.hpp>
#include <../Utilities/include/BooleanDesignVariableType.hpp>
#include <../Utilities/include/IntegerDesignVariableType.hpp>
#include <../Utilities/include/ContinuumDesignVariableNature.hpp>
#include <../Utilities/include/DiscreteDesignVariableNature.hpp>

// Objective Function Info Relavant Includes
#include <../Utilities/include/ObjectiveFunctionInfo.hpp>
#include <../Utilities/include/MinimizeObjectiveFunctionType.hpp>
#include <../Utilities/include/MaximizeObjectiveFunctionType.hpp>
#include <../Utilities/include/LinearObjectiveFunctionNature.hpp>
#include <../Utilities/include/SeekValueObjectiveFunctionType.hpp>
#include <../Utilities/include/SeekRangeObjectiveFunctionType.hpp>
#include <../Utilities/include/NonLinearObjectiveFunctionNature.hpp>

// Constraint Info Relavant Includes
#include <../Utilities/include/ConstraintInfo.hpp>
#include <../Utilities/include/EqualityConstraintType.hpp>
#include <../Utilities/include/LinearConstraintNature.hpp>
#include <../Utilities/include/NonLinearConstraintNature.hpp>
#include <../Utilities/include/NotEqualityConstraintType.hpp>
#include <../Utilities/include/TwoSidedInequalityConstraintType.hpp>






/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
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

bool
ConfigHelper::AddContinuumRealVariable(
    DesignTarget& target,
    const string& label,
    double lowerBound,
    double upperBound,
    int precision
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return target.AddDesignVariableInfo(
        *GetContinuumRealVariable(
            target, label, lowerBound, upperBound, precision
            )
        );
}

bool
ConfigHelper::AddDiscreteRealVariable(
    DesignTarget& target,
    const string& label,
    const JEGA::DoubleVector& values
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return target.AddDesignVariableInfo(
        *GetDiscreteRealVariable(target, label, values)
        );
}

bool
ConfigHelper::AddContinuumIntegerVariable(
    DesignTarget& target,
    const string& label,
    int lowerBound,
    int upperBound
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return target.AddDesignVariableInfo(
        *GetContinuumIntegerVariable(target, label, lowerBound, upperBound)
        );
}

bool
ConfigHelper::AddDiscreteIntegerVariable(
    DesignTarget& target,
    const string& label,
    const JEGA::IntVector& values
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return target.AddDesignVariableInfo(
        *GetDiscreteIntegerVariable(target, label, values)
        );
}

bool
ConfigHelper::AddBooleanVariable(
    JEGA::Utilities::DesignTarget& target,
    const std::string& label
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return target.AddDesignVariableInfo(
        *GetBooleanVariable(target, label)
        );
}

bool
ConfigHelper::AddLinearMinimizeObjective(
    DesignTarget& target,
    const string& label,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return target.AddObjectiveFunctionInfo(
        *GetLinearMinimizeObjective(target, label, coeffs)
        );
}

bool
ConfigHelper::AddLinearMaximizeObjective(
    DesignTarget& target,
    const string& label,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return target.AddObjectiveFunctionInfo(
        *GetLinearMaximizeObjective(target, label, coeffs)
        );
}

bool
ConfigHelper::AddLinearSeekValueObjective(
    DesignTarget& target,
    const string& label,
    double value,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return target.AddObjectiveFunctionInfo(
        *GetLinearSeekValueObjective(target, label, value, coeffs)
        );
}

bool
ConfigHelper::AddLinearSeekRangeObjective(
    DesignTarget& target,
    const string& label,
    double lowerBound,
    double upperBound,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return target.AddObjectiveFunctionInfo(
        *GetLinearSeekRangeObjective(
            target, label, lowerBound, upperBound, coeffs
            )
        );
}

bool
ConfigHelper::AddNonlinearMinimizeObjective(
    DesignTarget& target,
    const string& label
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return target.AddObjectiveFunctionInfo(
        *GetNonlinearMinimizeObjective(target, label)
        );
}

bool
ConfigHelper::AddNonlinearMaximizeObjective(
    DesignTarget& target,
    const string& label
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return target.AddObjectiveFunctionInfo(
        *GetNonlinearMaximizeObjective(target, label)
        );
}

bool
ConfigHelper::AddNonlinearSeekValueObjective(
    DesignTarget& target,
    const string& label,
    double value
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return target.AddObjectiveFunctionInfo(
        *GetNonlinearSeekValueObjective(target, label, value)
        );
}

bool
ConfigHelper::AddNonlinearSeekRangeObjective(
    DesignTarget& target,
    const string& label,
    double lowerBound,
    double upperBound
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return target.AddObjectiveFunctionInfo(
        *GetNonlinearSeekRangeObjective(target, label, lowerBound, upperBound)
        );
}

bool
ConfigHelper::AddLinearInequalityConstraint(
    DesignTarget& target,
    const string& label,
    double upperLimit,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return target.AddConstraintInfo(
        *GetLinearInequalityConstraint(target, label, upperLimit, coeffs)
        );
}

bool
ConfigHelper::AddLinearEqualityConstraint(
    DesignTarget& target,
    const string& label,
    double targetValue,
    double allowedViol,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return target.AddConstraintInfo(
        *GetLinearEqualityConstraint(
            target, label, targetValue, allowedViol, coeffs
            )
        );
}

bool
ConfigHelper::AddLinearNotEqualityConstraint(
    DesignTarget& target,
    const string& label,
    double tabooValue,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return target.AddConstraintInfo(
        *GetLinearNotEqualityConstraint(target, label, tabooValue, coeffs)
        );
}

bool
ConfigHelper::AddLinearTwoSidedInequalityConstraint(
    DesignTarget& target,
    const string& label,
    double lowerLimit,
    double upperLimit,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return target.AddConstraintInfo(
        *GetLinearTwoSidedInequalityConstraint(
            target, label, lowerLimit, upperLimit, coeffs
            )
        );
}

bool
ConfigHelper::AddNonlinearInequalityConstraint(
    DesignTarget& target,
    const string& label,
    double upperLimit
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return target.AddConstraintInfo(
        *GetNonlinearInequalityConstraint(target, label, upperLimit)
        );
}

bool
ConfigHelper::AddNonlinearEqualityConstraint(
    DesignTarget& target,
    const string& label,
    double targetValue,
    double allowedViol
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return target.AddConstraintInfo(
        *GetNonlinearEqualityConstraint(
            target, label, targetValue, allowedViol
            )
        );
}

bool
ConfigHelper::AddNonlinearNotEqualityConstraint(
    DesignTarget& target,
    const string& label,
    double tabooValue
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return target.AddConstraintInfo(
        *GetNonlinearNotEqualityConstraint(target, label, tabooValue)
        );
}

bool
ConfigHelper::AddNonlinearTwoSidedInequalityConstraint(
    DesignTarget& target,
    const string& label,
    double lowerLimit,
    double upperLimit
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return target.AddConstraintInfo(
        *GetNonlinearTwoSidedInequalityConstraint(
            target, label, lowerLimit, upperLimit
            )
        );
}

DesignVariableInfo*
ConfigHelper::GetContinuumRealVariable(
    DesignTarget& target,
    const std::string& label,
    double lowerBound,
    double upperBound,
    int precision
    )
{
    EDDY_FUNC_DEBUGSCOPE
    DesignVariableInfo* dvi = new DesignVariableInfo(target);
    dvi->SetLabel(label);

    dvi->SetType(new RealDesignVariableType(*dvi));
    dvi->SetNature(new ContinuumDesignVariableNature(dvi->GetType()));
    dvi->SetMinValue(lowerBound);
    dvi->SetMaxValue(upperBound);
    dvi->SetPrecision(precision);

    return dvi;
}

DesignVariableInfo*
ConfigHelper::GetDiscreteRealVariable(
    DesignTarget& target,
    const std::string& label,
    const JEGA::DoubleVector& values
    )
{
    EDDY_FUNC_DEBUGSCOPE
    DesignVariableInfo* dvi = new DesignVariableInfo(target);
    dvi->SetType(new RealDesignVariableType(*dvi));
    dvi->SetNature(new DiscreteDesignVariableNature(dvi->GetType()));

    dvi->SetLabel(label);
    dvi->AddDiscreteValues(values);

    return dvi;
}

DesignVariableInfo*
ConfigHelper::GetContinuumIntegerVariable(
    DesignTarget& target,
    const std::string& label,
    int lowerBound,
    int upperBound
    )
{
    EDDY_FUNC_DEBUGSCOPE
    DesignVariableInfo* dvi = new DesignVariableInfo(target);
    dvi->SetLabel(label);

    dvi->SetType(new IntegerDesignVariableType(*dvi));
    dvi->SetNature(new ContinuumDesignVariableNature(dvi->GetType()));
    dvi->SetMinValue(static_cast<double>(lowerBound));
    dvi->SetMaxValue(static_cast<double>(upperBound));

    return dvi;
}

DesignVariableInfo*
ConfigHelper::GetDiscreteIntegerVariable(
    DesignTarget& target,
    const std::string& label,
    const JEGA::IntVector& values
    )
{
    EDDY_FUNC_DEBUGSCOPE
    DesignVariableInfo* dvi = new DesignVariableInfo(target);
    dvi->SetType(new IntegerDesignVariableType(*dvi));
    dvi->SetNature(new DiscreteDesignVariableNature(dvi->GetType()));

    dvi->SetLabel(label);
    for(JEGA::IntVector::const_iterator it(values.begin());
        it!=values.end(); ++it)
            dvi->AddDiscreteValue(static_cast<double>(*it));

    return dvi;
}

DesignVariableInfo*
ConfigHelper::GetBooleanVariable(
    DesignTarget& target,
    const std::string& label
    )
{
    EDDY_FUNC_DEBUGSCOPE
    DesignVariableInfo* dvi = new DesignVariableInfo(target);
    dvi->SetType(new BooleanDesignVariableType(*dvi));
    dvi->SetNature(new DiscreteDesignVariableNature(dvi->GetType()));
    dvi->AddDiscreteValue(0.0);
    dvi->AddDiscreteValue(1.0);
    dvi->SetLabel(label);

    return dvi;
}

ObjectiveFunctionInfo*
ConfigHelper::GetLinearMinimizeObjective(
    DesignTarget& target,
    const std::string& label,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ObjectiveFunctionInfo* ofi = new ObjectiveFunctionInfo(target);
    ofi->SetLabel(label);
    ofi->SetType(new MinimizeObjectiveFunctionType(*ofi));
    ofi->SetNature(GetLinearObjectiveFunctionNature(ofi->GetType(), coeffs));
    return ofi;
}


ObjectiveFunctionInfo*
ConfigHelper::GetLinearMaximizeObjective(
    DesignTarget& target,
    const std::string& label,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ObjectiveFunctionInfo* ofi = new ObjectiveFunctionInfo(target);
    ofi->SetLabel(label);
    ofi->SetType(new MaximizeObjectiveFunctionType(*ofi));
    ofi->SetNature(GetLinearObjectiveFunctionNature(ofi->GetType(), coeffs));
    return ofi;
}

ObjectiveFunctionInfo*
ConfigHelper::GetLinearSeekValueObjective(
    DesignTarget& target,
    const std::string& label,
    double value,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ObjectiveFunctionInfo* ofi = new ObjectiveFunctionInfo(target);
    ofi->SetLabel(label);
    ofi->SetType(GetSeekValueOFType(*ofi, value));
    ofi->SetNature(GetLinearObjectiveFunctionNature(ofi->GetType(), coeffs));
    return ofi;
}

ObjectiveFunctionInfo*
ConfigHelper::GetLinearSeekRangeObjective(
    DesignTarget& target,
    const std::string& label,
    double lowerBound,
    double upperBound,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ObjectiveFunctionInfo* ofi = new ObjectiveFunctionInfo(target);
    ofi->SetLabel(label);
    ofi->SetType(GetSeekRangeOFType(*ofi, lowerBound, upperBound));
    ofi->SetNature(GetLinearObjectiveFunctionNature(ofi->GetType(), coeffs));
    return ofi;
}

ObjectiveFunctionInfo*
ConfigHelper::GetNonlinearMinimizeObjective(
    DesignTarget& target,
    const std::string& label
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ObjectiveFunctionInfo* ofi = new ObjectiveFunctionInfo(target);
    ofi->SetLabel(label);

    ofi->SetType(new MinimizeObjectiveFunctionType(*ofi));
    ofi->SetNature(new NonLinearObjectiveFunctionNature(ofi->GetType()));
    return ofi;
}

ObjectiveFunctionInfo*
ConfigHelper::GetNonlinearMaximizeObjective(
    DesignTarget& target,
    const std::string& label
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ObjectiveFunctionInfo* ofi = new ObjectiveFunctionInfo(target);
    ofi->SetLabel(label);

    ofi->SetType(new MaximizeObjectiveFunctionType(*ofi));
    ofi->SetNature(new NonLinearObjectiveFunctionNature(ofi->GetType()));
    return ofi;
}

ObjectiveFunctionInfo*
ConfigHelper::GetNonlinearSeekValueObjective(
    DesignTarget& target,
    const std::string& label,
    double value
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ObjectiveFunctionInfo* ofi = new ObjectiveFunctionInfo(target);
    ofi->SetLabel(label);
    ofi->SetType(GetSeekValueOFType(*ofi, value));

    ofi->SetNature(new NonLinearObjectiveFunctionNature(ofi->GetType()));
    return ofi;
}

ObjectiveFunctionInfo*
ConfigHelper::GetNonlinearSeekRangeObjective(
    DesignTarget& target,
    const std::string& label,
    double lowerBound,
    double upperBound
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ObjectiveFunctionInfo* ofi = new ObjectiveFunctionInfo(target);
    ofi->SetLabel(label);
    ofi->SetType(GetSeekRangeOFType(*ofi, lowerBound, upperBound));
    ofi->SetNature(new NonLinearObjectiveFunctionNature(ofi->GetType()));
    return ofi;
}

ConstraintInfo*
ConfigHelper::GetLinearInequalityConstraint(
    DesignTarget& target,
    const std::string& label,
    double upperLimit,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ConstraintInfo* cni = new ConstraintInfo(target);
    cni->SetLabel(label);
    cni->SetType(GetInequalityConstraintType(*cni, upperLimit));
    cni->SetNature(GetLinearConstraintNature(cni->GetType(), coeffs));
    return cni;
}

ConstraintInfo*
ConfigHelper::GetLinearEqualityConstraint(
    DesignTarget& target,
    const std::string& label,
    double targetValue,
    double allowedViol,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ConstraintInfo* cni = new ConstraintInfo(target);
    cni->SetLabel(label);
    cni->SetType(GetEqualityConstraintType(*cni, targetValue, allowedViol));
    cni->SetNature(GetLinearConstraintNature(cni->GetType(), coeffs));
    return cni;
}

ConstraintInfo*
ConfigHelper::GetLinearNotEqualityConstraint(
    DesignTarget& target,
    const std::string& label,
    double tabooValue,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ConstraintInfo* cni = new ConstraintInfo(target);
    cni->SetLabel(label);
    cni->SetType(GetNotEqualityConstraintType(*cni, tabooValue));
    cni->SetNature(GetLinearConstraintNature(cni->GetType(), coeffs));
    return cni;
}

ConstraintInfo*
ConfigHelper::GetLinearTwoSidedInequalityConstraint(
    DesignTarget& target,
    const std::string& label,
    double lowerLimit,
    double upperLimit,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ConstraintInfo* cni = new ConstraintInfo(target);
    cni->SetLabel(label);
    cni->SetType(
        GetTwoSidedInequalityConstraintType(*cni, lowerLimit, upperLimit)
        );
    cni->SetNature(GetLinearConstraintNature(cni->GetType(), coeffs));
    return cni;
}

ConstraintInfo*
ConfigHelper::GetNonlinearInequalityConstraint(
    DesignTarget& target,
    const std::string& label,
    double upperLimit
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ConstraintInfo* cni = new ConstraintInfo(target);
    cni->SetLabel(label);
    cni->SetType(GetInequalityConstraintType(*cni, upperLimit));
    cni->SetNature(new NonLinearConstraintNature(cni->GetType()));
    return cni;
}

ConstraintInfo*
ConfigHelper::GetNonlinearEqualityConstraint(
    DesignTarget& target,
    const std::string& label,
    double targetValue,
    double allowedViol
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ConstraintInfo* cni = new ConstraintInfo(target);
    cni->SetLabel(label);
    cni->SetType(GetEqualityConstraintType(*cni, targetValue, allowedViol));
    cni->SetNature(new NonLinearConstraintNature(cni->GetType()));
    return cni;
}

ConstraintInfo*
ConfigHelper::GetNonlinearNotEqualityConstraint(
    DesignTarget& target,
    const std::string& label,
    double tabooValue
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ConstraintInfo* cni = new ConstraintInfo(target);
    cni->SetLabel(label);
    cni->SetType(GetNotEqualityConstraintType(*cni, tabooValue));
    cni->SetNature(new NonLinearConstraintNature(cni->GetType()));
    return cni;
}

ConstraintInfo*
ConfigHelper::GetNonlinearTwoSidedInequalityConstraint(
    DesignTarget& target,
    const std::string& label,
    double lowerLimit,
    double upperLimit
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ConstraintInfo* cni = new ConstraintInfo(target);
    cni->SetLabel(label);
    cni->SetType(
        GetTwoSidedInequalityConstraintType(*cni, lowerLimit, upperLimit)
        );
    cni->SetNature(new NonLinearConstraintNature(cni->GetType()));
    return cni;
}


SeekRangeObjectiveFunctionType*
ConfigHelper::GetSeekRangeOFType(
    ObjectiveFunctionInfo& ofInfo,
    double lowerBound,
    double upperBound
    )
{
    EDDY_FUNC_DEBUGSCOPE
    SeekRangeObjectiveFunctionType* theType =
        new SeekRangeObjectiveFunctionType(ofInfo);

    theType->SetLowerBound(lowerBound);
    theType->SetUpperBound(upperBound);

    return theType;
}

SeekValueObjectiveFunctionType*
ConfigHelper::GetSeekValueOFType(
    ObjectiveFunctionInfo& ofInfo,
    double value
    )
{
    EDDY_FUNC_DEBUGSCOPE
    SeekValueObjectiveFunctionType* theType =
        new SeekValueObjectiveFunctionType(ofInfo);

    theType->SetValue(value);

    return theType;
}

LinearObjectiveFunctionNature*
ConfigHelper::GetLinearObjectiveFunctionNature(
    ObjectiveFunctionTypeBase& theType,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    LinearObjectiveFunctionNature* theNat =
        new LinearObjectiveFunctionNature(theType);

    theNat->SetCoefficients(coeffs);

    return theNat;
}


EqualityConstraintType*
ConfigHelper::GetEqualityConstraintType(
    ConstraintInfo& cnInfo,
    double targetValue,
    double allowedViol
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EqualityConstraintType* theType =
        new EqualityConstraintType(cnInfo);

    theType->SetTargetValue(targetValue);
    theType->SetAllowableViolation(allowedViol);

    return theType;
}

JEGA::Utilities::NotEqualityConstraintType*
ConfigHelper::GetNotEqualityConstraintType(
    JEGA::Utilities::ConstraintInfo& cnInfo,
    double tabooValue
    )
{
    EDDY_FUNC_DEBUGSCOPE
    NotEqualityConstraintType* theType =
        new NotEqualityConstraintType(cnInfo);
    theType->SetTabooValue(tabooValue);
    return theType;
}

InequalityConstraintType*
ConfigHelper::GetInequalityConstraintType(
    ConstraintInfo& cnInfo,
    double upperLimit
    )
{
    EDDY_FUNC_DEBUGSCOPE
    InequalityConstraintType* theType =
        new InequalityConstraintType(cnInfo);

    theType->SetUpperValue(upperLimit);

    return theType;
}

TwoSidedInequalityConstraintType*
ConfigHelper::GetTwoSidedInequalityConstraintType(
    ConstraintInfo& cnInfo,
    double lowerLimit,
    double upperLimit
    )
{
    EDDY_FUNC_DEBUGSCOPE
    TwoSidedInequalityConstraintType* theType =
        new TwoSidedInequalityConstraintType(cnInfo);

    theType->SetLowerValue(lowerLimit);
    theType->SetUpperValue(upperLimit);

    return theType;
}

LinearConstraintNature*
ConfigHelper::GetLinearConstraintNature(
    ConstraintTypeBase& theType,
    const JEGA::DoubleVector& coeffs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    LinearConstraintNature* theNat =
        new LinearConstraintNature(theType);

    theNat->SetCoefficients(coeffs);
    return theNat;
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








/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace FrontEnd
} // namespace JEGA

