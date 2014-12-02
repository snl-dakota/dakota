/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_JNI_EXTRAS_H
#define DAKOTA_JNI_EXTRAS_H

#define private public
#include "ProblemDescDB.hpp"
// #include "DakotaStrategy.hpp"
#include "DakotaModel.hpp"
#include "DakotaInterface.hpp"

namespace Dakota {
  DataMethodRep* dataMethodRep(DataMethod &dm);
  DataVariablesRep* dataVarsRep(DataVariables &dm);
  DataInterfaceRep* dataIfaceRep(DataInterface &dm);
  DataResponsesRep* dataRespRep(DataResponses &dm);

  std::vector<std::string> active_cr_variable_labels(const Variables& vars);
  std::vector<std::string> active_di_variable_labels(const Variables& vars);
  std::vector<std::string> active_dr_variable_labels(const Variables& vars);
};


#endif // DAKOTA_JNI_EXTRAS_H
