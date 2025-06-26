/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
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
