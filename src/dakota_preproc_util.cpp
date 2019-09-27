/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_preproc_util.hpp"
#include "WorkdirHelper.hpp"
#include "dakota_global_defs.hpp"

namespace Dakota {


std::string pyprepro_input(const std::string& template_file,
		           const std::string& preproc_cmd)
{
  std::string gen_inp = WorkdirHelper::system_tmp_file("dakota_input").string();
  std::string sys_cmd = preproc_cmd + " " + template_file + " " + gen_inp;

  Cout << "Preprocessing input with command:\n  " << sys_cmd << std::endl;
  int rc = std::system(sys_cmd.c_str());
  if (rc != 0) {
    Cerr << "\nError: Could not preprocess input with command:\n  " << sys_cmd
	 << "\nReturn code: " << rc << std::endl;
    abort_handler(IO_ERROR);
  }
  return gen_inp;
}


}  // namespace Dakota
