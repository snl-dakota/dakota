/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_JNI_UTILS_H
#define DAKOTA_JNI_UTILS_H

#include "ParallelLibrary.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaModel.hpp"
#include "DakotaInterface.hpp"

namespace DART {
  // Stub
  class DakotaFunctor {
  public:
    virtual void setContinuousRealVariables(std::vector<double>, std::vector<std::string>) = 0;
    virtual void setDiscreteIntegerVariables(std::vector<int>, std::vector<std::string>) = 0;
    virtual void setDiscreteRealVariables(std::vector<double>, std::vector<std::string>) = 0;
    virtual void setResponseLabels(std::vector<std::string>) = 0;
    virtual int evalFunction(std::vector<double>&, int evalId) = 0;
  };

  void connect_plugin(Dakota::ProblemDescDB *problem_db, DakotaFunctor *f);

  void clear_prp_cache();
};


#endif // DAKOTA_JNI_UTILS_H
