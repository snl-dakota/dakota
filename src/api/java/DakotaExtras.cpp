/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaExtras.hpp"

Dakota::DataMethodRep* Dakota::dataMethodRep(Dakota::DataMethod &dm) { return dm.dataMethodRep; }
Dakota::DataVariablesRep* Dakota::dataVarsRep(Dakota::DataVariables &dm) { return dm.dataVarsRep; }
Dakota::DataInterfaceRep* Dakota::dataIfaceRep(Dakota::DataInterface &dm) { return dm.dataIfaceRep; }
Dakota::DataResponsesRep* Dakota::dataRespRep(Dakota::DataResponses &dm) { return dm.dataRespRep; }

std::vector<std::string> Dakota::active_cr_variable_labels(const Variables& vars) {
  StringMultiArrayConstView view = vars.continuous_variable_labels();
  std::vector<std::string> result;
  for (int i=0; i<view.size(); ++i) {
    result.push_back(view[i].data());    
  }
  return result;
}

std::vector<std::string> Dakota::active_di_variable_labels(const Variables& vars) {
  StringMultiArrayConstView view = vars.discrete_int_variable_labels();
  std::vector<std::string> result;
  for (int i=0; i<view.size(); ++i) {
    result.push_back(view[i].data());    
  }
  return result;
}

std::vector<std::string> Dakota::active_dr_variable_labels(const Variables& vars) {
  StringMultiArrayConstView view = vars.discrete_real_variable_labels();
  std::vector<std::string> result;
  for (int i=0; i<view.size(); ++i) {
    result.push_back(view[i].data());    
  }
  return result;
}
