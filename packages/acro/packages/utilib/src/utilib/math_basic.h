/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

/**
 * \file math_basic.h
 *
 * Defines and constants for basic numerical operations.
 */

#ifndef utilib_math_basic_h
#define utilib_math_basic_h

#include <utilib/std_headers.h>
#include <utilib/_generic.h>


#ifdef UTILIB_SOLARIS_CC
#include <sunmath.h>
#include <ieeefp.h>
#endif

/*
 *
 * DEFINE MATHEMATICAL CONSTANTS
 *
 */
/** Value of the largest integer */
#ifndef MAXINT
#define MAXINT INT_MAX
#endif

/** Value of the largest float */
#ifndef MAXFLOAT
#define MAXFLOAT FLT_MAX
#endif

/** Value of the largest double */
#ifndef MAXDOUBLE
#define MAXDOUBLE DBL_MAX
#endif

/** Value of the largest long double */
#ifdef LDBL_MAX
#undef  MAXLONGDOUBLE
#define MAXLONGDOUBLE LDBL_MAX
#endif

#ifdef _MSC_VER
const long double M_PI = 3.1415926535897932384626433832795029L;
const long double M_E  = 2.7182818284590452354L;
#endif

#if 0
#if defined(DOXYGEN)
/* A definition for pi. */
const long double M_PI = 3.1415926535897932384626433832795029L;
/* A definition for e. */
const long double M_E  = 2.71828
#endif

/* TODO: Why these special definitions of PI?  What do we want
 * autoconf to test for?
 * Do we really want to try to define MAXINT when it is not defined
 * in a header file?
 * What is the purpose of the DOXYGEN sections?  Why bother?
 */
#if defined(DARWIN)
const double PI      = 3.1415926535897932384626433832795029L;

#elif !defined(UTILIB_AIX_CC)
const long double PI = 3.1415926535897932384626433832795029L;
#endif

#ifndef MAXINT
#define MAXINT (int)(~((int)0) ^ (1 << (sizeof(int)*8-1)))
#endif

/**
 * \def MAXINT
 *
 * The maximum integer value, which is system dependent.
 */
#if defined(DOXYGEN)
#ifdef MAXINT
#undef MAXINT
#endif
#define MAXINT
#endif

/**
 * \def PI
 *
 * The value of pi, which may be defined by the system.
 */
#if defined(DOXYGEN)
#ifdef PI
#undef PI
#endif
#define PI
#endif

/**
 * \def MAXFLOAT
 *
 * The maximum double value, which is system dependent.
 */
#if defined(DOXYGEN)
#ifdef MAXFLOAT
#undef MAXFLOAT
#endif
#define MAXFLOAT
#endif

/**
 * \def MAXDOUBLE
 *
 * The maximum double value, which is system dependent.
 */
#if defined(DOXYGEN)
#ifdef MAXDOUBLE
#undef MAXDOUBLE
#endif
#define MAXDOUBLE
#endif
#endif


#if defined(__cplusplus)
namespace utilib {

//#ifdef UTILIB_HAVE_NAMESPACES
//using std::max;
//using std::min;
//using std::swap;
//#endif

/*
 * 
 * OPERATIONS ON SIMPLE VALUES
 *
 */


/** Returns +1 if argument is positive, -1 if it is negative, and 0 otherwise. */
template<class T>
inline int sgn(const T& thing)
{
  if (thing > 0)
    return 1;
  if (thing < 0)
    return -1;
  return 0;
}

// convert any printable thing to a std::string
// used by pscientific
template <class T>
std::string tostring(const T &arg) {
  std::ostringstream out;
  out << arg;
  return(out.str());
  }


///
/// pscientific returns a portable scientific notation output, consistent
/// across platforms and bit sizes (within the machine precision).
/// stream options ios::uppercase and ios::showpos are supported
/// this does not support all stream options, and will have undetermined
/// results if flags such as left, right, hex, etc are set.
/// ios::setpoint and ios::width are not supported, but probably should be
///
std::string pscientific(const double t, int mantissa=6, int expwidth=3,
	std::ostream* os = NULL);

}

#endif

/*
 *
 * MISCELLANEOUS 
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Compute the number of lines in file filename. */
int calc_filesize(char* filename);

/**
 * Return the integer value of the rounded value of x.  
 * If the fractional part of x is less than 0.5, then x is
 * rounded down.  Otherwise, x is rounded up.
 */
#ifndef UTILIB_HAVE_LROUND
#ifndef DOXYGEN
long int lround _((double x));
#else
long int lround(double x);
#endif
#endif
#if defined(UTILIB_SGI_CC) || defined(UTILIB_OSF_CC) || defined(__PGI) || defined(_MSC_VER)
long int lround(double x);
#endif

/**
 * A method for rounding a double to num_digits
 * number of decimal digits after the decimal point.
 */
double d_round(double to_round, unsigned int num_digits);

#if 0
/* Setup the data structures for bufexp. */
#ifndef DOXYGEN
void setup_bufexp _((int tabsz, double xmin, double xmax));
#else
void setup_bufexp(int tabsz, double xmin, double xmax);
#endif

/* Compute the exponential function using fast buffered interpolation. */
#ifndef DOXYGEN
double bufexp _((double x));
#else
double bufexp(double x);
#endif
#endif


/* Compute number of bits needed to hold an integer value */
unsigned int bitWidth(unsigned int x);


/* Computes the greatest common divisor of two integers using the */
/* classical Euclidean algorithm (remainder version).             */
unsigned int gcd(unsigned int a, unsigned int b);


#ifdef __cplusplus
}
#endif

#if 0
/**
 * \def Exp(x)
 *
 * A macro masks the 'exp' function, possibly replacing it with bufexp to 
 * accelerate the calculation of 'exp'.
 *
 * TODO: Do we want BUFFERED_EXP to be determined by a configure
 *       command line option?
 */
#ifdef BUFFERED_EXP
#define Exp(x)	bufexp(x)
#else
#define Exp(x)	exp(x)
#endif
#endif

#endif


