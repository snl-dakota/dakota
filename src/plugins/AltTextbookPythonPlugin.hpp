#ifndef __ALTTEXTBOOK_PYTHON_PLUGIN_H__
#define __ALTTEXTBOOK_PYTHON_PLUGIN_H__

#include "DakotaPythonPlugin.hpp"

class AltTextbookPythonPlugin: public DakotaPlugins::DakotaPythonPlugin {

public:

  AltTextbookPythonPlugin() {};
  virtual ~AltTextbookPythonPlugin() {};

  DakotaPlugins::EvalResponse evaluate(
      DakotaPlugins::EvalRequest const& request) override;

  std::vector<DakotaPlugins::EvalResponse>
      evaluate(std::vector<DakotaPlugins::EvalRequest> const& requests)
      override;

private:

  void unpack_python_response(size_t const num_fns, size_t const num_derivs,
      pybind11::dict const& py_response,
      DakotaPlugins::EvalResponse& response);

};

#endif
