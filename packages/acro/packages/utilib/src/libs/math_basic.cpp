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

#include <utilib/math_basic.h>

namespace utilib {

std::string pscientific(const double t, int mantissa, int expwidth,
	std::ostream* os) {
  int my_exp=0;
  double my_t=t;
  bool neg, showpos, uc, negexp;
  std::string s;        // final number to print... as a string

  std::ios::fmtflags flags;

  if (os != NULL) {
    flags = os->flags();                // save current flags
    showpos = !!(flags & std::ios::showpos);
    uc = !!(flags & std::ios::uppercase );
    // mantissa = os->precision();      // if os passed, mantissa already set
    // width = os->width();             // nyi
  } else {                              // defaults if os not passed in
    showpos = false;
    uc = false;
  }

  // std::cout << mantissa << "," << expwidth << " pos/pnt/uc " << showpos << uc << std::endl;

  // work with positive numbers only
  if (my_t < 0.0) {
    neg = true;
    my_t = -my_t;
  } else
    neg = false;

  if (my_t != 0.0) {
    long double base10 = std::log10(static_cast<long double>(my_t));
    my_exp = static_cast<int>(base10);
    my_t = static_cast<double>(std::pow(static_cast<long double>(10), base10 - my_exp));
    // do nothing on 0.0 or error will occur
  }

// want to move decimal pt just past the first digit
// problem is the step function on the (int) log10(my_t)
  if (my_t > 0.0 && my_t < 1.0) {
    --my_exp;
    my_t *= 10.0;
  }
  while (my_t >= (10.0-std::pow(4.0,-1.0*(mantissa+1)))) {
	  my_t /= 10.0;
	  my_exp++;
  }

  if (my_exp < 0) {     // for ease of printing
    negexp = true;
    my_exp = -my_exp;
  } else
    negexp = false;

  std::string format;
  char* ss = new char [mantissa + expwidth + 10];

  // Ex: cout.setf(std::ios::showpos); cout << pscientific(.0123,4,3,&cout)
  format = neg?"-":(showpos?"+":"");		// +
  format += "%." + utilib::tostring(mantissa) + "f";	// +1.2300
  format += uc?'E':'e';				// +1.2300e
  format += negexp?'-':'+';			// +1.2300e-
  format += "%0" + utilib::tostring(expwidth) + 'd';	// +1.2300e-02
  // std::cout << format << endl;
#ifdef _MSC_VER
  sprintf_s(ss,mantissa+expwidth+10,format.c_str(),my_t,my_exp);
#else
  sprintf (ss, format.c_str(), my_t, my_exp);
#endif

  s = std::string(ss);
  delete ss;

//  if (os)			// restore flags
//    os->flags(flags);

  return s;
}

}

