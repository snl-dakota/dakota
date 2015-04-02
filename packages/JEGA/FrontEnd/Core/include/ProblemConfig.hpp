/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Front End

    CONTENTS:

        Definition of class ProblemConfig.

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

        Fri Jan 06 09:55:28 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the ProblemConfig class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_FRONTEND_JEGAPROBLEMCONFIG_HPP
#define JEGA_FRONTEND_JEGAPROBLEMCONFIG_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/JEGATypes.hpp>
#include <../Utilities/include/DesignTarget.hpp>






/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/





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
class ProblemConfig;







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
 * \brief A configuration object describing a problem to be solved by JEGA.
 *
 * This class houses information such as descriptions of the design variables,
 * objective functions, and constraints.
 *
 * It may seem a bit nonintuitive but this does not include the means of
 * evaluation.  This is only a description of the form of a problem.  The
 * means of evaluation is supplied throught the AlgorithmConfig class.
 * Thus, a problem configuration can be used with multiple algorithm
 * configurations and thus with multiple different evaluators.
 *
 * The only data structure housed by this problem configuration object is
 * a DesignTarget which is thread safe if threading is enabled.  Therefore
 * this class is threadsafe.
 */
class JEGA_SL_IEDECL ProblemConfig
{
    /*
    ============================================================================
    Class Scope Enumerations
    ============================================================================
    */
    public:


    protected:


    private:

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

        /// The DesignTarget in which the problem information is stored.
        /**
         * This will ultimately be the target used by the algorithms created
         * to solve this problem.  There may be multiple such algorithms and
         * they will have the ability to share in each other's discoveries.
         *
         * This is mutable because while it is owned by this object, it is not
         * up to this object to tell others when and how they can change it.
         * Therefore, it must be available for mutation at all times, including
         * when this config object is know by const reference.
         */
        mutable JEGA::Utilities::DesignTarget _theTarget;




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

        /**
         * \brief Allows mutable access to the DesignTarget contained within
         *        this object.
         *
         * \return A reference to the DesignTarget member data of this object.
         */
        inline
        JEGA::Utilities::DesignTarget&
        GetDesignTarget(
            ) const;

    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        void
        SetDiscardTracking(
            bool track
            );

        bool
        GetDiscardTracking(
            ) const;

        void
        SetMaxGuffSize(
            std::size_t mgs
            );

        std::size_t
        GetMaxGuffSize(
            ) const;

        /**
         * \brief Adds a continuum, real valued variable to the target
         *        using the supplied information.
         *
         * \param label The label for the new design variable description.
         * \param lowerBound The lowest legal value for the new design
         *                   variable.
         * \param upperBound The highest legal value for the new design
         *                   variable.
         * \param precision The decimal precision to be used by this new
         *                  variable (number of decimal places of interest).
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         ConfigHelper::AddContinuumRealVariable method.
         */
        bool
        AddContinuumRealVariable(
            const std::string& label,
            double lowerBound,
            double upperBound,
            int precision
            );

        /**
         * \brief Adds a discrete, real valued variable to the target
         *        using the supplied information.
         *
         * \param label The label for the new design variable description.
         * \param values The initial set of discrete values for this variable.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         ConfigHelper::AddDiscreteRealVariable method.
         */
        bool
        AddDiscreteRealVariable(
            const std::string& label,
            const JEGA::DoubleVector& values
            );

        /**
         * \brief Adds a continuum, integer valued variable to the
         *        target using the supplied information.
         *
         * \param label The label for the new design variable description.
         * \param lowerBound The lowest legal value for the new design
         *                   variable.
         * \param upperBound The highest legal value for the new design
         *                   variable.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         ConfigHelper::AddContinuumIntegerVariable method.
         */
        bool
        AddContinuumIntegerVariable(
            const std::string& label,
            int lowerBound,
            int upperBound
            );

        /**
         * \brief Adds a discrete, integer valued variable to the
         *        target using the supplied information.
         *
         * \param label The label for the new design variable description.
         * \param values The initial set of discrete values for this variable.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         ConfigHelper::AddDiscreteIntegerVariable method.
         */
        bool
        AddDiscreteIntegerVariable(
            const std::string& label,
            const JEGA::IntVector& values
            );

        /**
         * \brief Adds a Boolean valued variable to the target using
         *        the supplied information.
         *
         * \param label The label for the new design variable description.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         ConfigHelper::AddDesignVariableInfo method.
         */
        bool
        AddBooleanVariable(
            const std::string& label
            );

        /**
         * \brief Adds a linear, minimization objective to the
         *        target using the supplied information.
         *
         * \param label The label for the new objective function description.
         * \param coeffs The coefficients for the linear equation of this
         *               objective.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         ConfigHelper::AddLinearMinimizeObjective method.
         */
        bool
        AddLinearMinimizeObjective(
            const std::string& label,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Adds a linear, maximization objective to the
         *        target using the supplied information.
         *
         * \param label The label for the new objective function description.
         * \param coeffs The coefficients for the linear equation of this
         *               objective.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         ConfigHelper::AddLinearMaximizeObjective method.
         */
        bool
        AddLinearMaximizeObjective(
            const std::string& label,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Adds a linear, seek value objective to the
         *        target using the supplied information.
         *
         * \param label The label for the new objective function description.
         * \param value The sought value for this objective.
         * \param coeffs The coefficients for the linear equation of this
         *               objective.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         ConfigHelper::AddLinearSeekValueObjective method.
         */
        bool
        AddLinearSeekValueObjective(
            const std::string& label,
            double value,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Adds a linear, seek range objective to the
         *        target using the supplied information.
         *
         * \param label The label for the new objective function description.
         * \param lowerBound The lower bound of the range of sought values.
         * \param upperBound The upper bound of the range of sought values.
         * \param coeffs The coefficients for the linear equation of this
         *               objective.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         ConfigHelper::AddLinearSeekRangeObjective method.
         */
        bool
        AddLinearSeekRangeObjective(
            const std::string& label,
            double lowerBound,
            double upperBound,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Adds a non-linear, minimization objective to the
         *        target using the supplied information.
         *
         * \param label The label for the new objective function description.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         ConfigHelper::AddNonlinearMinimizeObjective method.
         */
        bool
        AddNonlinearMinimizeObjective(
            const std::string& label
            );

        /**
         * \brief Adds a non-linear, maximization objective to the
         *        target using the supplied information.
         *
         * \param label The label for the new objective function description.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         ConfigHelper::AddNonlinearMaximizeObjective method.
         */
        bool
        AddNonlinearMaximizeObjective(
            const std::string& label
            );

        /**
         * \brief Adds a non-linear, seek value objective to the
         *        target using the supplied information.
         *
         * \param label The label for the new objective function description.
         * \param value The sought value for this objective.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         ConfigHelper::AddNonlinearSeekValueObjective method.
         */
        bool
        AddNonlinearSeekValueObjective(
            const std::string& label,
            double value
            );

        /**
         * \brief Adds a non-linear, seek range objective to the
         *        target using the supplied information.
         *
         * \param label The label for the new objective function description.
         * \param lowerBound The lower bound of the range of sought values.
         * \param upperBound The upper bound of the range of sought values.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         ConfigHelper::AddNonlinearSeekRangeObjective method.
         */
        bool
        AddNonlinearSeekRangeObjective(
            const std::string& label,
            double lowerBound,
            double upperBound
            );

        /**
         * \brief Adds a linear, single-sided inequality constraint to the
         *        target using the supplied information.
         *
         * \param label The label for the new constraint description.
         * \param upperLimit The value above which the constraint is considered
         *                   to be violated.
         * \param coeffs The coefficients for the linear equation of this
         *               constraint.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         ConfigHelper::AddLinearInequalityConstraint method.
         */
        bool
        AddLinearInequalityConstraint(
            const std::string& label,
            double upperLimit = 0.0,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Adds a linear, equality constraint to the target
         *        using the supplied information.
         *
         * \param label The label for the new constraint description.
         * \param target The value away from which the constraint is
         *               considered to be violated.
         * \param allowedViol The amount by which strict equality can be
         *                    violated without infeasibility.
         * \param coeffs The coefficients for the linear equation of this
         *               constraint.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         ConfigHelper::AddLinearEqualityConstraint method.
         */
        bool
        AddLinearEqualityConstraint(
            const std::string& label,
            double target,
            double allowedViol = 0.0,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Adds a linear, not-equality constraint to the supplied target
         *        using the supplied information.
         *
         * \param label The label for the new constraint description.
         * \param tabooValue The value at which the constraint is considered to
         *                   be violated.
         * \param coeffs The coefficients for the linear equation of this
         *               constraint.  If not supplied, the created info object
         *               will be unable to evaluate itself.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         ConfigHelper::AddLinearNotEqualityConstraint method.
         */
        bool
        AddLinearNotEqualityConstraint(
            const std::string& label,
            double tabooValue,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Adds a linear, two-sided inequality constraint to the
         *        target using the supplied information.
         *
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
         *         ConfigHelper::AddLinearTwoSidedInequalityConstraint method.
         */
        bool
        AddLinearTwoSidedInequalityConstraint(
            const std::string& label,
            double lowerLimit,
            double upperLimit,
            const JEGA::DoubleVector& coeffs = JEGA::DoubleVector()
            );

        /**
         * \brief Adds a non-linear, single-sided inequality constraint to the
         *        target using the supplied information.
         *
         * \param label The label for the new constraint description.
         * \param upperLimit The value above which the constraint is considered
         *                   to be violated.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         ConfigHelper::AddNonlinearInequalityConstraint method.
         */
        bool
        AddNonlinearInequalityConstraint(
            const std::string& label,
            double upperLimit = 0.0
            );

        /**
         * \brief Adds a non-linear, equality constraint to the target
         *        using the supplied information.
         *
         * \param label The label for the new constraint description.
         * \param target The value away from which the constraint is
         *               considered to be violated.
         * \param allowedViol The amount by which strict equality can be
         *                    violated without infeasibility.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         ConfigHelper::AddNonlinearEqualityConstraint method.
         */
        bool
        AddNonlinearEqualityConstraint(
            const std::string& label,
            double target,
            double allowedViol = 0.0
            );

        /**
         * \brief Adds a non-linear, equality constraint to the supplied target
         *        using the supplied information.
         *
         * \param label The label for the new constraint description.
         * \param tabooValue The value at which the constraint is considered to
         *                   be violated.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         ConfigHelper::AddNonlinearNotEqualityConstraint method.
         */
        bool
        AddNonlinearNotEqualityConstraint(
            const std::string& label,
            double tabooValue
            );

        /**
         * \brief Adds a non-linear, two-sided inequality constraint to the
         *        target using the supplied information.
         *
         * \param label The label for the new constraint description.
         * \param lowerLimit The value below which the constraint is considered
         *                   to be violated.
         * \param upperLimit The value above which the constraint is considered
         *                   to be violated.
         * \return True if the new description is added successfully and false
         *         if not.  This comes directly from the
         *         ConfigHelper::AddNonlinearTwoSidedInequalityConstraint
         *         method.
         */
        bool
        AddNonlinearTwoSidedInequalityConstraint(
            const std::string& label,
            double lowerLimit,
            double upperLimit
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
    public:

        /// Default constructs a ProblemConfig object.
        ProblemConfig(
            );


}; // class ProblemConfig



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
#include "./inline/ProblemConfig.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_FRONTEND_JEGAPROBLEMCONFIG_HPP
