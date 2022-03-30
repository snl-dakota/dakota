#ifndef __TEXTBOOK_PYTHON_PLUGIN_H__
#define __TEXTBOOK_PYTHON_PLUGIN_H__

#include "DakotaPythonPlugin.hpp"

class TextbookPythonPlugin: public DakotaPlugins::DakotaPythonPlugin {

public:

  TextbookPythonPlugin() {};
  virtual ~TextbookPythonPlugin() {};

  DakotaPlugins::EvalResponse evaluate(
      DakotaPlugins::EvalRequest const& request) override;

};

#endif
