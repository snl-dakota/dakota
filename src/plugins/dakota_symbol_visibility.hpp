/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SYMBOL_VISIBILITY_H
#define DAKOTA_SYMBOL_VISIBILITY_H

// RATIONALE: Boost config.hpp offers a more general/portable
// implementation than the basics herein, but Dakota plugins shouldn't
// require Boost. If Boost used, most any version of Boost would
// suffice.

#ifdef DAKOTA_PLUGINS_USE_BOOST
#  include <boost/config.hpp>
#  define DAKOTA_SYMBOL_EXPORT BOOST_SYMBOL_EXPORT
#else
#  if (defined(_WIN32) || defined(__WIN32__) || defined(WIN32)) && !defined(__CYGWIN__)
#    define DAKOTA_SYMBOL_EXPORT __attribute__((__dllexport__))
#  elif defined(__INTEL_COMPILER)
#    define DAKOTA_SYMBOL_EXPORT __attribute__((visibility("default")))
#  else
//   gcc, clang
#    define DAKOTA_SYMBOL_EXPORT __attribute__((__visibility__("default")))
#  endif
#endif

#endif
