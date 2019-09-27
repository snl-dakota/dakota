/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_PREPROC_UTIL_H
#define DAKOTA_PREPROC_UTIL_H

#include <string>

namespace Dakota {


// write input string to tmp file

// dprepro to a tmp file

// echo the raw, then processed file

/// run pyprepro on the user-provided input file and return generated tmp output
std::string pyprepro_input(const std::string& template_file,
		           const std::string& preproc_cmd = "pyprepro.py");


}  // namespace Dakota

#endif // include guard
