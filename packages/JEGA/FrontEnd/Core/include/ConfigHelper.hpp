/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Front End

    CONTENTS:

        Definition of class ConfigHelper.

    NOTES:

        See notes under Class Definition section of this file.

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
 * \brief Contains the definition of the ConfigHelper class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_FRONTEND_JEGA_FEUTILS_HPP
#define JEGA_FRONTEND_JEGA_FEUTILS_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <string>
#include <../Utilities/include/JEGATypes.hpp>





/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/
namespace JEGA
{
    namespace Utilities
    {
        class DesignTarget;
        class ConstraintInfo;
        class ConstraintTypeBase;
        class DesignVariableInfo;
        class ObjectiveFunctionInfo;
        class EqualityConstraintType;
        class LinearConstraintNature;
        class InequalityConstraintType;
        class NotEqualityConstraintType;
        class ObjectiveFunctionTypeBase;
        class LinearObjectiveFunctionNature;
        class SeekValueObjectiveFunctionType;
        class SeekRangeObjectiveFunctionType;
        class TwoSidedInequalityConstraintType;
    }
}






/*
================================================================================
Namespace Aliases
================================================================================
*/








/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace FrontEnd {





/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
class ConfigHelper;







/*
================================================================================
In-Namespace File Scope Typedefs
================================================================================
*/








/*
================================================================================
Class Definition
================================================================================
*/
/**
 * \brief A helper class for loading problem configuration objects.
 *
 * This class has methods for loading variables, objectives, and constraints
 * into a DesignTarget.
 */
class JEGA_SL_IEDECL ConfigHelper
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:





    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /**
         * \brief Adds a continuum, real valued variable to the supplied target
         *        using the supplied information.
         *
         * \param target The DesignTarget into which to add the new variable
         *               description.
         * \param label The label for the new design variable description.
         * \param lowerBound The lowest legal value for the new design
         *                   variable.
         * \param upperBound The highest legal value for the new design
         *                   variable.
         * \param precision The decimal precision to be used by this new
         *                  variable (number of decimal places of interest).
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         DesignTarget::AddDesignVariableInfo method.
         */
        static
        bool
        AddContinuumRealVariable(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double lowerBound,
            double upperBound,
            int precision
            );

        /**
         * \brief Adds a discrete, real valued variable to the supplied target
         *        using the supplied information.
         *
         * \param target The DesignTarget into which to add the new variable
         *               description.
         * \param label The label for the new design variable description.
         * \param values The initial set of discrete values for this variable.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         DesignTarget::AddDesignVariableInfo method.
         */
        static
        bool
        AddDiscreteRealVariable(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            const JEGA::DoubleVector& values
            );

        /**
         * \brief Adds a continuum, integer valued variable to the supplied
         *        target using the supplied information.
         *
         * \param target The DesignTarget into which to add the new variable
         *               description.
         * \param label The label for the new design variable description.
         * \param lowerBound The lowest legal value for the new design
         *                   variable.
         * \param upperBound The highest legal value for the new design
         *                   variable.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         DesignTarget::AddDesignVariableInfo method.
         */
        static
        bool
        AddContinuumIntegerVariable(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            int lowerBound,
            int upperBound
            );

        /**
         * \brief Adds a discrete, integer valued variable to the supplied
         *        target using the supplied information.
         *
         * \param target The DesignTarget into which to add the new variable
         *               description.
         * \param label The label for the new design variable description.
         * \param values The initial set of discrete values for this variable.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         DesignTarget::AddDesignVariableInfo method.
         */
        static
        bool
        AddDiscreteIntegerVariable(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            const JEGA::IntVector& values
            );

        /**
         * \brief Adds a Boolean valued variable to the supplied target using
         *        the supplied information.
         *
         * \param target The DesignTarget into which to add the new variable
         *               description.
         * \param label The label for the new design variable description.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         DesignTarget::AddDesignVariableInfo method.
         */
        static
        bool
        AddBooleanVariable(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label
            );

        /**
         * \brief Adds a linear, minimization objective to the supplied
         *        target using the supplied information.
         *
         * \param target The DesignTarget into which to add the new objective
         *               description.
         * \param label The label for the new objective function description.
         * \param coeffs The coefficients for the linear equation of this
         *               objective.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         DesignTarget::AddObjectiveFunctionInfo method.
         */
        static
        bool
        AddLinearMinimizeObjective(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Adds a linear, maximization objective to the supplied
         *        target using the supplied information.
         *
         * \param target The DesignTarget into which to add the new objective
         *               description.
         * \param label The label for the new objective function description.
         * \param coeffs The coefficients for the linear equation of this
         *               objective.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         DesignTarget::AddObjectiveFunctionInfo method.
         */
        static
        bool
        AddLinearMaximizeObjective(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Adds a linear, seek value objective to the supplied
         *        target using the supplied information.
         *
         * \param target The DesignTarget into which to add the new objective
         *               description.
         * \param label The label for the new objective function description.
         * \param value The sought value for this objective.
         * \param coeffs The coefficients for the linear equation of this
         *               objective.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         DesignTarget::AddObjectiveFunctionInfo method.
         */
        static
        bool
        AddLinearSeekValueObjective(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double value,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Adds a linear, seek range objective to the supplied
         *        target using the supplied information.
         *
         * \param target The DesignTarget into which to add the new objective
         *               description.
         * \param label The label for the new objective function description.
         * \param lowerBound The lower bound of the range of sought values.
         * \param upperBound The upper bound of the range of sought values.
         * \param coeffs The coefficients for the linear equation of this
         *               objective.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         DesignTarget::AddObjectiveFunctionInfo method.
         */
        static
        bool
        AddLinearSeekRangeObjective(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double lowerBound,
            double upperBound,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Adds a non-linear, minimization objective to the supplied
         *        target using the supplied information.
         *
         * \param target The DesignTarget into which to add the new objective
         *               description.
         * \param label The label for the new objective function description.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         DesignTarget::AddObjectiveFunctionInfo method.
         */
        static
        bool
        AddNonlinearMinimizeObjective(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label
            );

        /**
         * \brief Adds a non-linear, maximization objective to the supplied
         *        target using the supplied information.
         *
         * \param target The DesignTarget into which to add the new objective
         *               description.
         * \param label The label for the new objective function description.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         DesignTarget::AddObjectiveFunctionInfo method.
         */
        static
        bool
        AddNonlinearMaximizeObjective(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label
            );

        /**
         * \brief Adds a non-linear, seek value objective to the supplied
         *        target using the supplied information.
         *
         * \param target The DesignTarget into which to add the new objective
         *               description.
         * \param label The label for the new objective function description.
         * \param value The sought value for this objective.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         DesignTarget::AddObjectiveFunctionInfo method.
         */
        static
        bool
        AddNonlinearSeekValueObjective(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double value
            );

        /**
         * \brief Adds a non-linear, seek range objective to the supplied
         *        target using the supplied information.
         *
         * \param target The DesignTarget into which to add the new objective
         *               description.
         * \param label The label for the new objective function description.
         * \param lowerBound The lower bound of the range of sought values.
         * \param upperBound The upper bound of the range of sought values.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         DesignTarget::AddObjectiveFunctionInfo method.
         */
        static
        bool
        AddNonlinearSeekRangeObjective(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double lowerBound,
            double upperBound
            );

        /**
         * \brief Adds a linear, single-sided inequality constraint to the
         *        supplied target using the supplied information.
         *
         * \param target The DesignTarget into which to add the new constraint
         *               description.
         * \param label The label for the new constraint description.
         * \param upperLimit The value above which the constraint is considered
         *                   to be violated.
         * \param coeffs The coefficients for the linear equation of this
         *               constraint.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         DesignTarget::AddConstraintInfo method.
         */
        static
        bool
        AddLinearInequalityConstraint(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double upperLimit = 0.0,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Adds a linear, equality constraint to the supplied target
         *        using the supplied information.
         *
         * \param target The DesignTarget into which to add the new constraint
         *               description.
         * \param label The label for the new constraint description.
         * \param targetValue The value away from which the constraint is
         *                    considered to be violated.
         * \param allowedViol The amount by which strict equality can be
         *                    violated without infeasibility.
         * \param coeffs The coefficients for the linear equation of this
         *               constraint.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         DesignTarget::AddConstraintInfo method.
         */
        static
        bool
        AddLinearEqualityConstraint(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double targetValue,
            double allowedViol = 0.0,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Adds a linear, not-equality constraint to the supplied target
         *        using the supplied information.
         *
         * \param target The DesignTarget into which to add the new constraint
         *               description.
         * \param label The label for the new constraint description.
         * \param tabooValue The value at which the constraint is considered to
         *                   be violated.
         * \param coeffs The coefficients for the linear equation of this
         *               constraint.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         DesignTarget::AddConstraintInfo method.
         */
        static
        bool
        AddLinearNotEqualityConstraint(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double tabooValue,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Adds a linear, two-sided inequality constraint to the
         *        supplied target using the supplied information.
         *
         * \param target The DesignTarget into which to add the new constraint
         *               description.
         * \param label The label for the new constraint description.
         * \param lowerLimit The value below which the constraint is considered
         *                   to be violated.
         * \param upperLimit The value above which the constraint is considered
         *                   to be violated.
         * \param coeffs The coefficients for the linear equation of this
         *               constraint.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         DesignTarget::AddConstraintInfo method.
         */
        static
        bool
        AddLinearTwoSidedInequalityConstraint(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double lowerLimit,
            double upperLimit,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Adds a non-linear, single-sided inequality constraint to the
         *        supplied target using the supplied information.
         *
         * \param target The DesignTarget into which to add the new constraint
         *               description.
         * \param label The label for the new constraint description.
         * \param upperLimit The value above which the constraint is considered
         *                   to be violated.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         DesignTarget::AddConstraintInfo method.
         */
        static
        bool
        AddNonlinearInequalityConstraint(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double upperLimit = 0.0
            );

        /**
         * \brief Adds a non-linear, equality constraint to the supplied target
         *        using the supplied information.
         *
         * \param target The DesignTarget into which to add the new constraint
         *               description.
         * \param label The label for the new constraint description.
         * \param targetValue The value away from which the constraint is
         *                    considered to be violated.
         * \param allowedViol The amount by which strict equality can be
         *                    violated without infeasibility.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         DesignTarget::AddConstraintInfo method.
         */
        static
        bool
        AddNonlinearEqualityConstraint(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double targetValue,
            double allowedViol = 0.0
            );

        /**
         * \brief Adds a non-linear, equality constraint to the supplied target
         *        using the supplied information.
         *
         * \param target The DesignTarget into which to add the new constraint
         *               description.
         * \param label The label for the new constraint description.
         * \param tabooValue The value at which the constraint is considered to
         *                   be violated.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         DesignTarget::AddConstraintInfo method.
         */
        static
        bool
        AddNonlinearNotEqualityConstraint(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double tabooValue
            );

        /**
         * \brief Adds a non-linear, two-sided inequality constraint to the
         *        supplied target using the supplied information.
         *
         * \param target The DesignTarget into which to add the new constraint
         *               description.
         * \param label The label for the new constraint description.
         * \param lowerLimit The value below which the constraint is considered
         *                   to be violated.
         * \param upperLimit The value above which the constraint is considered
         *                   to be violated.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         DesignTarget::AddConstraintInfo method.
         */
        static
        bool
        AddNonlinearTwoSidedInequalityConstraint(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double lowerLimit,
            double upperLimit
            );

        /**
         * \brief Returns a continuum, real valued variable info.
         *
         * The returned info object must subsequently be added to a
         * DesignTarget at which point it will receive it's index.  The target
         * to which it is added must be the target supplied to this method.
         * Use the corresponding Add method to do this in a single step.
         *
         * \param target The DesignTarget for which to create the new variable
         *               description.
         * \param label The label for the new design variable description.
         * \param lowerBound The lowest legal value for the new design
         *                   variable.
         * \param upperBound The highest legal value for the new design
         *                   variable.
         * \param precision The decimal precision to be used by this new
         *                  variable (number of decimal places of interest).
         * \return The newly created DesignVariableInfo object.
         */
        static
        JEGA::Utilities::DesignVariableInfo*
        GetContinuumRealVariable(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double lowerBound,
            double upperBound,
            int precision
            );

        /**
         * \brief Returns a discrete, real valued variable info.
         *
         * The returned info object must subsequently be added to a
         * DesignTarget at which point it will receive it's index.  The target
         * to which it is added must be the target supplied to this method.
         * Use the corresponding Add method to do this in a single step.
         *
         * \param target The DesignTarget for which to create the new variable
         *               description.
         * \param label The label for the new design variable description.
         * \param values The initial set of discrete values for this variable.
         * \return The newly created DesignVariableInfo object.
         */
        static
        JEGA::Utilities::DesignVariableInfo*
        GetDiscreteRealVariable(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            const JEGA::DoubleVector& values
            );

        /**
         * \brief Returns a continuum, integer valued variable info.
         *
         * The returned info object must subsequently be added to a
         * DesignTarget at which point it will receive it's index.  The target
         * to which it is added must be the target supplied to this method.
         * Use the corresponding Add method to do this in a single step.
         *
         * \param target The DesignTarget for which to create the new variable
         *               description.
         * \param label The label for the new design variable description.
         * \param lowerBound The lowest legal value for the new design
         *                   variable.
         * \param upperBound The highest legal value for the new design
         *                   variable.
         * \return The newly created DesignVariableInfo object.
         */
        static
        JEGA::Utilities::DesignVariableInfo*
        GetContinuumIntegerVariable(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            int lowerBound,
            int upperBound
            );

        /**
         * \brief Returns a discrete, integer valued variable info.
         *
         * The returned info object must subsequently be added to a
         * DesignTarget at which point it will receive it's index.  The target
         * to which it is added must be the target supplied to this method.
         * Use the corresponding Add method to do this in a single step.
         *
         * \param target The DesignTarget for which to create the new variable
         *               description.
         * \param label The label for the new design variable description.
         * \param values The initial set of discrete values for this variable.
         * \return The newly created DesignVariableInfo object.
         */
        static
        JEGA::Utilities::DesignVariableInfo*
        GetDiscreteIntegerVariable(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            const JEGA::IntVector& values
            );

        /**
         * \brief Returns a Boolean valued variable info.
         *
         * The returned info object must subsequently be added to a
         * DesignTarget at which point it will receive it's index.  The target
         * to which it is added must be the target supplied to this method.
         * Use the corresponding Add method to do this in a single step.
         *
         * \param target The DesignTarget for which to create the new variable
         *               description.
         * \param label The label for the new design variable description.
         * \return The newly created DesignVariableInfo object.
         */
        static
        JEGA::Utilities::DesignVariableInfo*
        GetBooleanVariable(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label
            );

        /**
         * \brief Returns a linear, minimization objective info.
         *
         * The returned info object must subsequently be added to a
         * DesignTarget at which point it will receive it's index.  The target
         * to which it is added must be the target supplied to this method.
         * Use the corresponding Add method to do this in a single step.
         *
         * \param target The DesignTarget for which to create the new objective
         *               description.
         * \param label The label for the new objective function description.
         * \param coeffs The coefficients for the linear equation of this
         *               objective.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return The newly created ObjectiveFunctionInfo object.
         */
        static
        JEGA::Utilities::ObjectiveFunctionInfo*
        GetLinearMinimizeObjective(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Returns a linear, maximization objective info.
         *
         * The returned info object must subsequently be added to a
         * DesignTarget at which point it will receive it's index.  The target
         * to which it is added must be the target supplied to this method.
         * Use the corresponding Add method to do this in a single step.
         *
         * \param target The DesignTarget for which to create the new objective
         *               description.
         * \param label The label for the new objective function description.
         * \param coeffs The coefficients for the linear equation of this
         *               objective.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return The newly created ObjectiveFunctionInfo object.
         */
        static
        JEGA::Utilities::ObjectiveFunctionInfo*
        GetLinearMaximizeObjective(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Returns a linear, seek value objective info.
         *
         * The returned info object must subsequently be added to a
         * DesignTarget at which point it will receive it's index.  The target
         * to which it is added must be the target supplied to this method.
         * Use the corresponding Add method to do this in a single step.
         *
         * \param target The DesignTarget for which to create the new objective
         *               description.
         * \param label The label for the new objective function description.
         * \param value The sought value for this objective.
         * \param coeffs The coefficients for the linear equation of this
         *               objective.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return The newly created ObjectiveFunctionInfo object.
         */
        static
        JEGA::Utilities::ObjectiveFunctionInfo*
        GetLinearSeekValueObjective(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double value,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Returns a linear, seek range objective info.
         *
         * The returned info object must subsequently be added to a
         * DesignTarget at which point it will receive it's index.  The target
         * to which it is added must be the target supplied to this method.
         * Use the corresponding Add method to do this in a single step.
         *
         * \param target The DesignTarget for which to create the new objective
         *               description.
         * \param label The label for the new objective function description.
         * \param lowerBound The lower bound of the range of sought values.
         * \param upperBound The upper bound of the range of sought values.
         * \param coeffs The coefficients for the linear equation of this
         *               objective.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return The newly created ObjectiveFunctionInfo object.
         */
        static
        JEGA::Utilities::ObjectiveFunctionInfo*
        GetLinearSeekRangeObjective(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double lowerBound,
            double upperBound,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Returns a non-linear, minimization objective info.
         *
         * The returned info object must subsequently be added to a
         * DesignTarget at which point it will receive it's index.  The target
         * to which it is added must be the target supplied to this method.
         * Use the corresponding Add method to do this in a single step.
         *
         * \param target The DesignTarget for which to create the new objective
         *               description.
         * \param label The label for the new objective function description.
         * \return The newly created ObjectiveFunctionInfo object.
         */
        static
        JEGA::Utilities::ObjectiveFunctionInfo*
        GetNonlinearMinimizeObjective(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label
            );

        /**
         * \brief Returns a non-linear, maximization objective info.
         *
         * The returned info object must subsequently be added to a
         * DesignTarget at which point it will receive it's index.  The target
         * to which it is added must be the target supplied to this method.
         * Use the corresponding Add method to do this in a single step.
         *
         * \param target The DesignTarget for which to create the new objective
         *               description.
         * \param label The label for the new objective function description.
         * \return The newly created ObjectiveFunctionInfo object.
         */
        static
        JEGA::Utilities::ObjectiveFunctionInfo*
        GetNonlinearMaximizeObjective(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label
            );

        /**
         * \brief Returns a non-linear, seek value objective info.
         *
         * The returned info object must subsequently be added to a
         * DesignTarget at which point it will receive it's index.  The target
         * to which it is added must be the target supplied to this method.
         * Use the corresponding Add method to do this in a single step.
         *
         * \param target The DesignTarget for which to create the new objective
         *               description.
         * \param label The label for the new objective function description.
         * \param value The sought value for this objective.
         * \return The newly created ObjectiveFunctionInfo object.
         */
        static
        JEGA::Utilities::ObjectiveFunctionInfo*
        GetNonlinearSeekValueObjective(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double value
            );

        /**
         * \brief Returns a non-linear, seek range objective info.
         *
         * The returned info object must subsequently be added to a
         * DesignTarget at which point it will receive it's index.  The target
         * to which it is added must be the target supplied to this method.
         * Use the corresponding Add method to do this in a single step.
         *
         * \param target The DesignTarget for which to create the new objective
         *               description.
         * \param label The label for the new objective function description.
         * \param lowerBound The lower bound of the range of sought values.
         * \param upperBound The upper bound of the range of sought values.
         * \return The newly created ObjectiveFunctionInfo object.
         */
        static
        JEGA::Utilities::ObjectiveFunctionInfo*
        GetNonlinearSeekRangeObjective(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double lowerBound,
            double upperBound
            );

        /**
         * \brief Returns a linear, single-sided inequality constraint info.
         *
         * The returned info object must subsequently be added to a
         * DesignTarget at which point it will receive it's index.  The target
         * to which it is added must be the target supplied to this method.
         * Use the corresponding Add method to do this in a single step.
         *
         * \param target The DesignTarget for which to create the new
         *               constraint description.
         * \param label The label for the new constraint description.
         * \param upperLimit The value above which the constraint is considered
         *                   to be violated.
         * \param coeffs The coefficients for the linear equation of this
         *               constraint.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return The newly created ConstraintInfo object.
         */
        static
        JEGA::Utilities::ConstraintInfo*
        GetLinearInequalityConstraint(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double upperLimit = 0.0,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Returns a linear, equality constraint info.
         *
         * The returned info object must subsequently be added to a
         * DesignTarget at which point it will receive it's index.  The target
         * to which it is added must be the target supplied to this method.
         * Use the corresponding Add method to do this in a single step.
         *
         * \param target The DesignTarget for which to create the new
         *               constraint description.
         * \param label The label for the new constraint description.
         * \param targetValue The value away from which the constraint is
         *                    considered to be violated.
         * \param allowedViol The amount by which strict equality can be
         *                    violated without infeasibility.
         * \param coeffs The coefficients for the linear equation of this
         *               constraint.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return The newly created ConstraintInfo object.
         */
        static
        JEGA::Utilities::ConstraintInfo*
        GetLinearEqualityConstraint(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double targetValue,
            double allowedViol = 0.0,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Returns a linear, not-equality constraint info.
         *
         * The returned info object must subsequently be added to a
         * DesignTarget at which point it will receive it's index.  The target
         * to which it is added must be the target supplied to this method.
         * Use the corresponding Add method to do this in a single step.
         *
         * \param target The DesignTarget for which to create the new
         *               constraint description.
         * \param label The label for the new constraint description.
         * \param tabooValue The value at which the constraint is considered to
         *                   be violated.
         * \param coeffs The coefficients for the linear equation of this
         *               constraint.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return The newly created ConstraintInfo object.
         */
        static
        JEGA::Utilities::ConstraintInfo*
        GetLinearNotEqualityConstraint(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double tabooValue,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Returns a linear, two-sided inequality constraint info.
         *
         * The returned info object must subsequently be added to a
         * DesignTarget at which point it will receive it's index.  The target
         * to which it is added must be the target supplied to this method.
         * Use the corresponding Add method to do this in a single step.
         *
         * \param target The DesignTarget for which to create the new
         *               constraint description.
         * \param label The label for the new constraint description.
         * \param lowerLimit The value below which the constraint is considered
         *                   to be violated.
         * \param upperLimit The value above which the constraint is considered
         *                   to be violated.
         * \param coeffs The coefficients for the linear equation of this
         *               constraint.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return The newly created ConstraintInfo object.
         */
        static
        JEGA::Utilities::ConstraintInfo*
        GetLinearTwoSidedInequalityConstraint(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double lowerLimit,
            double upperLimit,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Returns a non-linear, single-sided inequality constraint
         *        info.
         *
         * The returned info object must subsequently be added to a
         * DesignTarget at which point it will receive it's index.  The target
         * to which it is added must be the target supplied to this method.
         * Use the corresponding Add method to do this in a single step.
         *
         * \param target The DesignTarget for which to create the new
         *               constraint description.
         * \param label The label for the new constraint description.
         * \param upperLimit The value above which the constraint is considered
         *                   to be violated.
         * \return The newly created ConstraintInfo object.
         */
        static
        JEGA::Utilities::ConstraintInfo*
        GetNonlinearInequalityConstraint(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double upperLimit = 0.0
            );

        /**
         * \brief Returns a non-linear, equality constraint info.
         *
         * The returned info object must subsequently be added to a
         * DesignTarget at which point it will receive it's index.  The target
         * to which it is added must be the target supplied to this method.
         * Use the corresponding Add method to do this in a single step.
         *
         * \param target The DesignTarget for which to create the new
         *               constraint description.
         * \param label The label for the new constraint description.
         * \param targetValue The value away from which the constraint is
         *                    considered to be violated.
         * \param allowedViol The amount by which strict equality can be
         *                    violated without infeasibility.
         * \return The newly created ConstraintInfo object.
         */
        static
        JEGA::Utilities::ConstraintInfo*
        GetNonlinearEqualityConstraint(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double targetValue,
            double allowedViol = 0.0
            );

        /**
         * \brief Returns a non-linear, equality constraint info.
         *
         * The returned info object must subsequently be added to a
         * DesignTarget at which point it will receive it's index.  The target
         * to which it is added must be the target supplied to this method.
         * Use the corresponding Add method to do this in a single step.
         *
         * \param target The DesignTarget for which to create the new
         *               constraint description.
         * \param label The label for the new constraint description.
         * \param tabooValue The value at which the constraint is considered to
         *                   be violated.
         * \return The newly created ConstraintInfo object.
         */
        static
        JEGA::Utilities::ConstraintInfo*
        GetNonlinearNotEqualityConstraint(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double tabooValue
            );

        /**
         * \brief Returns a non-linear, two-sided inequality constraint info.
         *
         * The returned info object must subsequently be added to a
         * DesignTarget at which point it will receive it's index.  The target
         * to which it is added must be the target supplied to this method.
         * Use the corresponding Add method to do this in a single step.
         *
         * \param target The DesignTarget for which to create the new
         *               constraint description.
         * \param label The label for the new constraint description.
         * \param lowerLimit The value below which the constraint is considered
         *                   to be violated.
         * \param upperLimit The value above which the constraint is considered
         *                   to be violated.
         * \return The newly created ConstraintInfo object.
         */
        static
        JEGA::Utilities::ConstraintInfo*
        GetNonlinearTwoSidedInequalityConstraint(
            JEGA::Utilities::DesignTarget& target,
            const std::string& label,
            double lowerLimit,
            double upperLimit
            );

        /**
         * \brief Creates and returns a new SeekRangeObjectiveFunctionType
         *        built using the supplied parameters.
         *
         * The returned type object will not have been set in the info object
         * using the SetType method.  That must be done externally to this
         * method.
         *
         * \param ofInfo The objective function info for which this type is
         *               being created.
         * \param lowerBound The lower bound of the range of sought values.
         * \param upperBound The upper bound of the range of sought values.
         * \return A newly created seek range objective type.
         */
        static
        JEGA::Utilities::SeekRangeObjectiveFunctionType*
        GetSeekRangeOFType(
            JEGA::Utilities::ObjectiveFunctionInfo& ofInfo,
            double lowerBound,
            double upperBound
            );

        /**
         * \brief Creates and returns a new SeekValueObjectiveFunctionType
         *        built using the supplied parameters.
         *
         * The returned type object will not have been set in the info object
         * using the SetType method.  That must be done externally to this
         * method.
         *
         * \param ofInfo The objective function info for which this type is
         *               being created.
         * \param value The sought value for this objective.
         * \return A newly created seek value objective type.
         */
        static
        JEGA::Utilities::SeekValueObjectiveFunctionType*
        GetSeekValueOFType(
            JEGA::Utilities::ObjectiveFunctionInfo& ofInfo,
            double value
            );

        /**
         * \brief Creates and returns a new LinearObjectiveFunctionNature
         *        built using the supplied parameters.
         *
         * The returned nature object will not have been set in the type object
         * using the SetNature method.  That must be done externally to this
         * method.
         *
         * \param theType The objective function type for which this nature is
         *                being created.
         * \param coeffs The coefficients for the linear equation of this
         *               objective.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return A newly created linear objective nature.
         */
        static
        JEGA::Utilities::LinearObjectiveFunctionNature*
        GetLinearObjectiveFunctionNature(
            JEGA::Utilities::ObjectiveFunctionTypeBase& theType,
            const JEGA::DoubleVector& coeffs
            );

        /**
         * \brief Creates and returns a new EqualityConstraintType
         *        built using the supplied parameters.
         *
         * The returned type object will not have been set in the info object
         * using the SetType method.  That must be done externally to this
         * method.
         *
         * \param cnInfo The constraint info for which this type is
         *               being created.
         * \param targetValue The sought value for this constraint.
         * \param allowedViol The amount by which strict equality can be
         *                    violated without infeasibility.
         * \return A newly created equality constraint type.
         */
        static
        JEGA::Utilities::EqualityConstraintType*
        GetEqualityConstraintType(
            JEGA::Utilities::ConstraintInfo& cnInfo,
            double targetValue,
            double allowedViol = 0.0
            );

        /**
         * \brief Creates and returns a new NotEqualityConstraintType
         *        built using the supplied parameters.
         *
         * The returned type object will not have been set in the info object
         * using the SetType method.  That must be done externally to this
         * method.
         *
         * \param cnInfo The constraint info for which this type is being
         *               created.
         * \param tabooValue The taboo value for this constraint.
         * \return A newly created not-equality constraint type.
         */
        static
        JEGA::Utilities::NotEqualityConstraintType*
        GetNotEqualityConstraintType(
            JEGA::Utilities::ConstraintInfo& cnInfo,
            double tabooValue
            );

        /**
         * \brief Creates and returns a new InequalityConstraintType
         *        built using the supplied parameters.
         *
         * The returned type object will not have been set in the info object
         * using the SetType method.  That must be done externally to this
         * method.
         *
         * \param cnInfo The constraint info for which this type is being
         *               created.
         * \param upperLimit The value above which the constraint is considered
         *                   to be violated.
         * \return A newly created inequality constraint type.
         */
        static
        JEGA::Utilities::InequalityConstraintType*
        GetInequalityConstraintType(
            JEGA::Utilities::ConstraintInfo& cnInfo,
            double upperLimit
            );

        /**
         * \brief Creates and returns a new TwoSidedInequalityConstraintType
         *        built using the supplied parameters.
         *
         * The returned type object will not have been set in the info object
         * using the SetType method.  That must be done externally to this
         * method.
         *
         * \param cnInfo The constraint info for which this type is being
         *               created.
         * \param lowerLimit The value below which the constraint is considered
         *                   to be violated.
         * \param upperLimit The value above which the constraint is considered
         *                   to be violated.
         * \return A newly created two-sided inequality constraint type.
         */
        static
        JEGA::Utilities::TwoSidedInequalityConstraintType*
        GetTwoSidedInequalityConstraintType(
            JEGA::Utilities::ConstraintInfo& cnInfo,
            double lowerLimit,
            double upperLimit
            );

        /**
         * \brief Creates and returns a new LinearConstraintNature
         *        built using the supplied parameters.
         *
         * The returned nature object will not have been set in the type object
         * using the SetNature method.  That must be done externally to this
         * method.
         *
         * \param theType The constraint type for which this nature is
         *                being created.
         * \param coeffs The coefficients for the linear equation of this
         *               constraint.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return A newly created linear constraint nature.
         */
        static
        JEGA::Utilities::LinearConstraintNature*
        GetLinearConstraintNature(
            JEGA::Utilities::ConstraintTypeBase& theType,
            const JEGA::DoubleVector& coeffs
            );

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:







    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:





    /*
    ============================================================================
    Structors
    ============================================================================
    */
    private:

        /// This constructor is private and has no implementation.
        /**
         * This class has nothing but static methods and thus shouldn't be
         * instantiated.
         */
        ConfigHelper(
            );




}; // class ConfigHelper



/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace FrontEnd
} // namespace JEGA







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
#include "inline/ConfigHelper.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_FRONTEND_JEGA_FEUTILS_HPP
