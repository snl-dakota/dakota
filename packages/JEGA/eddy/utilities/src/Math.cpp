/*
================================================================================
    PROJECT:

        Eddy C++ Utilities Project

    CONTENTS:

        Implementation of class Math.

    NOTES:

        See notes of Math.hpp.

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
 * \brief Contains the implementation of the Math class.
 */




/*
================================================================================
Includes
================================================================================
*/
#include <cmath>
#include <cfloat>
#include <cstddef>
#include <cstdlib>
#include <cassert>
#include "../include/Math.hpp"





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
namespace eddy {
    namespace utilities {







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

double
Math::Floor(
    double arg
    )
{
    return floor(arg);

} // Math::Floor

float
Math::Floor(
    float arg
    )
{
    return static_cast<float>(floor(static_cast<double>(arg)));

} // Math::Floor

double
Math::Ceil(
    double arg
    )
{
    return ceil(arg);

} // Math::Ceil

float
Math::Ceil(
    float arg
    )
{
    return static_cast<float>(ceil(static_cast<double>(arg)));

} // Math::Ceil

double
Math::Exp(
    double arg
    )
{
    return exp(arg);

} // Math::Exp

float
Math::Exp(
    float arg
    )
{
    return static_cast<float>(exp(static_cast<double>(arg)));

} // Math::Exp

double
Math::Round(
    double arg,
    int prec
    )
{
    double conv = prec == 0.0 ? 1.0 : pow(10.0, static_cast<double>(prec));
    return (arg > 0.0) ?
                floor(arg * conv + 0.5) / conv :
                ceil (arg * conv - 0.5) / conv;

} // Math::Round

float
Math::Round(
    float arg,
    int prec
    )
{
    float conv = prec == 0.0f ? 1.0f : pow(10.0f, static_cast<float>(prec));
    return (arg > 0.0) ?
                floor(arg * conv + 0.5f) / conv :
                ceil (arg * conv - 0.5f) / conv;

} // Math::Round

double
Math::Truncate(
    double arg,
    int prec
    )
{
    if(prec == 0) return static_cast<double>(static_cast<int>(arg));
    double conv = pow(10.0, static_cast<double>(prec));
    return (arg > 0.0) ?
                floor(arg * conv) / conv :
                ceil (arg * conv) / conv;

} // Math::Truncate

float
Math::Truncate(
    float arg,
    int prec
    )
{
    if(prec == 0) return static_cast<float>(static_cast<int>(arg));
    float conv = pow(10.0f, static_cast<float>(prec));
    return (arg > 0.0) ?
                floor(arg * conv) / conv :
                ceil (arg * conv) / conv;

} // Math::Truncate

double
Math::Sqrt(
    double arg
    )
{
    return sqrt(arg);

} // Math::Sqrt

float
Math::Sqrt(
    float arg
    )
{
    return static_cast<float>(sqrt(static_cast<double>(arg)));

} // Math::Sqrt

double
Math::Log(
    double arg,
    double base
    )
{
    return (base==10.0) ? log10(arg) : (log10(arg) / log10(base));

} // Math::Log

float
Math::Log(
    float arg,
    float base
    )
{
    return (base==10.0f) ? Log10(arg) : (Log10(arg) / Log10(base));

} // Math::Log

double
Math::Log10(
    double arg
    )
{
    return log10(arg);

} // Math::Log10

float
Math::Log10(
    float arg
    )
{
    return static_cast<float>(log10(static_cast<double>(arg)));

} // Math::Log10

double
Math::Ln(
    double arg
    )
{
    return log(arg);

} // Math::Ln

float
Math::Ln(
    float arg
    )
{
    return static_cast<float>(log(static_cast<double>(arg)));

} // Math::Ln

double
Math::Pow(
    double arg,
    double exp
    )
{
    return pow(arg, exp);

} // Math::Pow

float
Math::Pow(
    float arg,
    float exp
    )
{
    return static_cast<float>(
        pow(static_cast<double>(arg), static_cast<double>(exp))
        );

} // Math::Pow

double
Math::Abs(
    double arg
    )
{
    return fabs(arg);

} // Math::Abs

int
Math::Abs(
    int arg
    )
{
    return abs(arg);

} // Math::Abs

long
Math::Abs(
    long arg
    )
{
    return labs(arg);

} // Math::Abs

float
Math::Abs(
    float arg
    )
{
    return static_cast<float>(fabs(static_cast<double>(arg)));

} // Math::Abs

double
Math::Sin(
    double arg
    )
{
    return sin(arg);

} // Math::Sin

float
Math::Sin(
    float arg
    )
{
    return static_cast<float>(sin(static_cast<double>(arg)));

} // Math::Sin

double
Math::Cos(
    double arg
    )
{
    return cos(arg);

} // Math::Cos

float
Math::Cos(
    float arg
    )
{
    return static_cast<float>(cos(static_cast<double>(arg)));

} // Math::Cos

double
Math::Tan(
    double arg
    )
{
    return tan(arg);

} // Math::Tan

float
Math::Tan(
    float arg
    )
{
    return static_cast<float>(tan(static_cast<double>(arg)));

} // Math::Tan

double
Math::ArcSin(
    double arg
    )
{
    return asin(arg);

} // Math::ArcSin

float
Math::ArcSin(
    float arg
    )
{
    return static_cast<float>(asin(static_cast<double>(arg)));

} // Math::ArcSin

double
Math::ArcCos(
    double arg
    )
{
    return acos(arg);

} // Math::ArcCos

float
Math::ArcCos(
    float arg
    )
{
    return static_cast<float>(acos(static_cast<double>(arg)));

} // Math::ArcCos

double
Math::ArcTan(
    double arg
    )
{
    return atan(arg);

} // Math::ArcTan

float
Math::ArcTan(
    float arg
    )
{
    return static_cast<float>(atan(static_cast<double>(arg)));

} // Math::ArcTan

double
Math::LambertW(
    const double arg
    )
{
    if(0.0 == arg) return 0.0;

    const double eps = 4.0e-16;
    const double em1 = 0.3678794411714423215955237701614608;

    assert(arg>=-em1);

    double p,e,t,w;

    if(arg < (-em1+1e-4))
    {
        // series near -em1 in sqrt(q)
        double q = arg+em1;
        double r=Sqrt(q);
        double q2=q*q;
        double q3=q2*q;

        return
            -1.0
            +2.331643981597124203363536062168*r
            -1.812187885639363490240191647568*q
            +1.936631114492359755363277457668*r*q
            -2.353551201881614516821543561516*q2
            +3.066858901050631912893148922704*r*q2
            -4.175335600258177138854984177460*q3
            +5.858023729874774148815053846119*r*q3
            -8.401032217523977370984161688514*q3*q;  // error approx 1e-16
    }

    /* initial approx for iteration... */
    if(arg < 1.0)
    {
        /* series near 0 */
        p = Sqrt(2.0 * (2.7182818284590452353602874713526625*arg + 1.0));
        w = -1.0 + p * (1.0 + p *
            (-0.333333333333333333333 + p * 0.152777777777777777777777)
            );
    }
    else w=Ln(arg); /* asymptotic */

    if(arg > 3.0) w -= Ln(w); /* useful? */

    for (unsigned int i=0; i<10; ++i)
    {
        /* Halley iteration */
        e = Exp(w);
        t = w * e - arg;
        p = w + 1.0;
        t /= e * p - 0.5 * (p + 1.0) * t / p;
        w -= t;
        if (Abs(t) < eps * (1.0 + Abs(w))) return w; /* rel-abs error */
    }

    assert(false && "Lambert W - No convergence.");
    return 0.0f;

} // Math::LambertW

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
    } // namespace utilities
} // namespace eddy
