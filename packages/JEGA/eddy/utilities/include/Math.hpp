/*
================================================================================
    PROJECT:

        Eddy C++ Utilities Project

    CONTENTS:

        Definition of class Math.

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

        Tue Jun 03 10:30:43 2003 - Original Version (JE)

================================================================================
*/



/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the Math class.
 */


/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_UTILITIES_MATH_HPP
#define EDDY_UTILITIES_MATH_HPP







/*
================================================================================
Includes
================================================================================
*/
#include "config.hpp"






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
namespace eddy {
    namespace utilities {







/*
================================================================================
Forward Declares
================================================================================
*/
class Math;







/*
================================================================================
Class Definition
================================================================================
*/

/// Contains many commonly used math functions
/**
 * This class provides a common interface for different math functions
 * and is intended to be portable.  Use this to avoid problems for example
 * where the max and min functions are not defined.
 *
 * All methods are static so it is not necessary and thus not allowed
 * to create an instance of the Math class.
 */
class EDDY_SL_IEDECL Math
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





    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:





    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns true if the supplied number is a whole number.
        /**
         * \param val The value to test as a whole number or not.
         * \return True if val has no decimal portion and false otherwise.
         */
        inline static
        bool
        IsWhole(
            double val
            );

        /// Returns true if the supplied number is a whole number.
        /**
         * \param val The value to test as a whole number or not.
         * \return True if val has no decimal portion and false otherwise.
         */
        inline static
        bool
        IsWhole(
            float val
            );

        /// Returns the largest whole number not greater than \a val.
        /**
         * \param val The value to find the floor of.
         * \return The floor of \a val.
         */
        static
        double
        Floor(
            double val
            );

        /// Returns the largest whole number not greater than \a val.
        /**
         * \param val The value to find the floor of.
         * \return The floor of \a val.
         */
        static
        float
        Floor(
            float val
            );

        /// Returns the smallest whole number not smaller than \a val.
        /**
         * \param val The value to find the ceiling of.
         * \return The ceiling of \a val.
         */
        static
        double
        Ceil(
            double val
            );

        /// Returns the smallest whole number not smaller than \a val.
        /**
         * \param val The value to find the ceiling of.
         * \return The ceiling of \a val.
         */
        static
        float
        Ceil(
            float val
            );

        /// Returns the ratio of a circles circumference to its diameter.
        /**
         * \return The value 3.1415926535897932384 as an approximation to Pi.
         */
        inline static
        double
        Pi(
            );

        /// Returns e^val.
        /**
         * The default is to return the exponential constant itself:
         * 2.718281828459...
         *
         * \param val The exponent value for the computation.
         * \return e raised to the power of \a val.
         */
        static
        double
        Exp(
            double val = 1.0
            );

        /// Returns the e^val.
        /**
         * The default is to return the exponential constant itself:
         * 2.718281828459...
         *
         * \param val The exponent value for the computation.
         * \return e raised to the power of \a val.
         */
        static
        float
        Exp(
            float val = 1.0f
            );

        /// Rounds \a val to \a prec decimal places.
        /**
         * Positive numbers cause rounding to the right of the radix.
         * For example, \a prec = 2 will cause rounding to the nearest
         * hundredth.
         *
         * Negative numbers cause rounding to the left of the radix.
         * For example, \a prec = -1 will cause rounding to the nearest 10.
         *
         * The default precision causes rounding to the nearest whole number.
         *
         * \param val The value to be rounded.
         * \param prec The precision to which \a val is to be rounded.
         * \return val rounded to \a prec decimal places.
         */
        static
        double
        Round(
            double val,
            int prec = 0
            );

        /// Rounds \a val to \a prec decimal places.
        /**
         * Positive numbers cause rounding to the right of the radix.
         * For example, \a prec = 2 will cause rounding to the nearest
         * hundredth.
         *
         * Negative numbers cause rounding to the left of the radix.
         * For example, \a prec = -1 will cause rounding to the nearest 10.
         *
         * The default precision causes rounding to the nearest whole number.
         *
         * \param val The value to be rounded.
         * \param prec The precision to which \a val is to be rounded.
         * \return val rounded to \a prec decimal places.
         */
        static
        float
        Round(
            float val,
            int prec = 0
            );

        /// Truncates \a val at \a prec decimal places.
        /**
         * Negative numbers cause truncation to the left of the radix.
         * For example, \a prec = -1 will cause truncation to the nearest 10.
         *
         * The default precision causes truncation to the nearest whole number.
         *
         * \param val The value to be truncated.
         * \param prec The precision to which \a val is to be truncated.
         * \return val trunctated at the \a prec decimal place.
         */
        static
        double
        Truncate(
            double val,
            int prec = 0
            );

        /// Truncates \a val to \a prec decimal places.
        /**
         * Negative numbers cause truncation to the left of the radix.
         * For example, \a prec = -1 will cause truncation to the nearest 10.
         *
         * The default precision causes truncation to the nearest whole number.
         *
         * \param val The value to be truncated.
         * \param prec The precision to which \a val is to be truncated.
         * \return val trunctated at the \a prec decimal place.
         */
        static
        float
        Truncate(
            float val,
            int prec = 0
            );

        /// Returns the square root of \a val.
        /**
         * Passing \a val as a negative number causes an assertion failure.
         *
         * \param val The value to take the square root of.
         * \return The square root of val.
         */
        static
        double
        Sqrt(
            double val
            );

        /// Returns the square root of \a val.
        /**
         * Passing \a val as a negative number causes an assertion failure.
         *
         * \param val The value to take the square root of.
         * \return The square root of val.
         */
        static
        float
        Sqrt(
            float val
            );

        /// Returns log base \a base of \a val.
        /**
         * For example, Log(x, 10) is the same as the call Log10(x),
         * Log(x, Exp()) is the same as Ln(x), Log(x, 2.0) is log-base-2
         * of x, etc.
         *
         * \param val The value to take the log of in base \a base.
         * \param base The base of the logarithm to apply to \a val.
         * \return The log of \a val in base \a base.
         */
        static
        double
        Log(
            double val,
            double base
            );

        /// Returns log base \a base of \a val.
        /**
         * For example, Log(x, 10) is the same as the call Log10(x),
         * Log(x, Exp()) is the same as Ln(x), Log(x, 2.0) is log-base-2
         * of x, etc.
         *
         * \param val The value to take the log of in base \a base.
         * \param base The base of the logarithm to apply to \a val.
         * \return The log of \a val in base \a base.
         */
        static
        float
        Log(
            float val,
            float base
            );

        /// Returns log base 10 of \a val.
        /**
         * \param val The value to take the log of in base 10.
         * \return The log of \a val in base 10.
         */
        static
        double
        Log10(
            double val
            );

        /// Returns log base 10 of \a val.
        /**
         * \param val The value to take the log of in base 10.
         * \return The log of \a val in base 10.
         */
        static
        float
        Log10(
            float val
            );

        /// Returns the natural log of val.
        /**
         * \param val The value to take the log of in base e (Exp()).
         * \return The log of \a val in base e (Exp()).
         */
        static
        double
        Ln(
            double val
            );

        /// Returns the natural log of val.
        /**
         * \param val The value to take the log of in base e (Exp()).
         * \return The log of \a val in base e (Exp()).
         */
        static
        float
        Ln(
            float val
            );

        /// Returns \a val raised to the power of \a exp.
        /**
         * \param val The value to raise to the \a exp power.
         * \param exp The exponent to which to raise \a val.
         * \return val^exp.
         */
        static
        double
        Pow(
            double val,
            double exp
            );

        /// Returns \a val raised to the power of \a exp.
        /**
         * \param val The value to raise to the \a exp power.
         * \param exp The exponent to which to raise \a val.
         * \return val^exp.
         */
        static
        float
        Pow(
            float val,
            float exp
            );

        /// Returns the sine of \a val.
        /**
         * \param val The value to take the sine of.
         * \return sin(val).
         */
        static
        double
        Sin(
            double val
            );

        /// Returns the sine of \a val.
        /**
         * \param val The value to take the sine of.
         * \return sin(val).
         */
        static
        float
        Sin(
            float val
            );

        /// Returns the cosine of \a val.
        /**
         * \param val The value to take the cosine of.
         * \return cos(val).
         */
        static
        double
        Cos(
            double val
            );

        /// Returns the cosine of \a val.
        /**
         * \param val The value to take the cosine of.
         * \return cos(val).
         */
        static
        float
        Cos(
            float val
            );

        /// Returns the tangent of \a val.
        /**
         * \param val The value to take the tangent of.
         * \return tan(val).
         */
        static
        float
        Tan(
            float val
            );

        /// Returns the tangent of \a val.
        /**
         * \param val The value to take the tangent of.
         * \return tan(val).
         */
        static
        double
        Tan(
            double val
            );

        /// Returns the inverse sine of \a val.
        /**
         * \param val The value to take the arc sine of.
         * \return asin(val).
         */
        static
        double
        ArcSin(
            double val
            );

        /// Returns the inverse sine of \a val.
        /**
         * \param val The value to take the arc sine of.
         * \return asin(val).
         */
        static
        float
        ArcSin(
            float val
            );

        /// Returns the inverse cosine of \a val.
        /**
         * \param val The value to take the arc cosine of.
         * \return acos(val).
         */
        static
        double
        ArcCos(
            double val
            );

        /// Returns the inverse cosine of \a val.
        /**
         * \param val The value to take the arc cosine of.
         * \return acos(val).
         */
        static
        float
        ArcCos(
            float val
            );

        /// Returns the inverse tangent of \a val.
        /**
         * \param val The value to take the arc tangent of.
         * \return atan(val).
         */
        static
        double
        ArcTan(
            double val
            );

        /// Returns the inverse tangent of \a val.
        /**
         * \param val The value to take the arc tangent of.
         * \return atan(val).
         */
        static
        float
        ArcTan(
            float val
            );

        /// Returns the larger of arg1 and arg2.
        /**
         * This method uses the < operator so only types for
         * which it is supported can be used.
         *
         * \param arg1 The first argument to compare for the max.
         * \param arg2 The second argument to compare for the max.
         * \return The larger of arg1 and arg2.
         */
        template<typename T>
        inline static
        const T&
        Max(
            const T& arg1,
            const T& arg2
            );

        /// Returns the smaller of arg1 and arg2.
        /**
         * This method uses the < operator so only types for
         * which it is supported can be used.
         *
         * \param arg1 The first argument to compare for the min.
         * \param arg2 The second argument to compare for the min.
         * \return The smaller of arg1 and arg2.
         */
        template<typename T>
        inline static
        const T&
        Min(
            const T& arg1,
            const T& arg2
            );

        /// Returns the absolute value of val.
        /**
         * \param val The value to take the absolute value of.
         * \return abs(val).
         */
        static
        double
        Abs(
            double val
            );

        /// Returns the absolute value of val.
        /**
         * \param val The value to take the absolute value of.
         * \return abs(val).
         */
        static
        int
        Abs(
            int val
            );

        /// Returns the absolute value of val.
        /**
         * \param val The value to take the absolute value of.
         * \return abs(val).
         */
        static
        long
        Abs(
            long val
            );

        /// Returns the absolute value of val.
        /**
         * \param val The value to take the absolute value of.
         * \return abs(val).
         */
        static
        float
        Abs(
            float val
            );

        /// Returns the LambertW of val.
        /**
         * The Lambert W function is the inverse function of x*Exp(x).
         *
         * \param val The point at which to calculate the lambertW function.
         * \return lambertW(val).
         */
        double
        LambertW(
            const double val
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


    private:

        /// This constructor is private and has no implementation.
        /**
         * This class has nothing but static methods and is not meant to be
         * instantiated.
         */
        Math(
            );



}; // class Math



/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace utilities
} // namespace eddy







/*
================================================================================
Include Inlined Methods File
================================================================================
*/
#include "./inline/Math.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_UTILITIES_MATH_HPP
