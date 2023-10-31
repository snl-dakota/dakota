/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_PREPROC_UTIL_H
#define DAKOTA_PREPROC_UTIL_H

#include <string>

namespace Dakota {


/// utility to write an input string to a tmpfile in PWD
std::string string_to_tmpfile(const std::string& dump_string);

/// run pyprepro on the user-provided input file and return generated tmp output
std::string pyprepro_input(const std::string& template_file,
		           const std::string& preproc_cmd = "pyprepro.py");


}  // namespace Dakota

#endif // include guard
