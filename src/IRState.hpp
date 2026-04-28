/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_IR_STATE_H
#define DAKOTA_IR_STATE_H

#include "IRStore.hpp"

#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace Dakota {

enum class IRBlockType {
  Environment,
  Method,
  Model,
  Variables,
  Interface,
  Responses
};

struct IRActiveSelection {
  size_t method{0};
  size_t model{0};
  size_t variables{0};
  size_t interface{0};
  size_t responses{0};
};

/// Complete materialized IR state across all top-level blocks.
struct IRState {
  IRStore environment;
  std::vector<IRStore> method;
  std::vector<IRStore> model;
  std::vector<IRStore> variables;
  std::vector<IRStore> interface;
  std::vector<IRStore> responses;

  IRActiveSelection active;

  // Optional ID -> index maps for string-based active-node setters.
  std::unordered_map<String, size_t> method_id_to_index;
  std::unordered_map<String, size_t> model_id_to_index;
  std::unordered_map<String, size_t> variables_id_to_index;
  std::unordered_map<String, size_t> interface_id_to_index;
  std::unordered_map<String, size_t> responses_id_to_index;

  IRStore& active_store(IRBlockType block)
  {
    switch (block) {
    case IRBlockType::Environment: return environment;
    case IRBlockType::Method: return method.at(active.method);
    case IRBlockType::Model: return model.at(active.model);
    case IRBlockType::Variables: return variables.at(active.variables);
    case IRBlockType::Interface: return interface.at(active.interface);
    case IRBlockType::Responses: return responses.at(active.responses);
    }
    throw std::runtime_error("IRState::active_store: unknown block");
  }

  const IRStore& active_store(IRBlockType block) const
  {
    return const_cast<IRState*>(this)->active_store(block);
  }

  bool set_active_method(const String& id)
  {
    auto it = method_id_to_index.find(id);
    if (it == method_id_to_index.end()) return false;
    active.method = it->second;
    return true;
  }

  bool set_active_model(const String& id)
  {
    auto it = model_id_to_index.find(id);
    if (it == model_id_to_index.end()) return false;
    active.model = it->second;
    return true;
  }

  bool set_active_variables(const String& id)
  {
    auto it = variables_id_to_index.find(id);
    if (it == variables_id_to_index.end()) return false;
    active.variables = it->second;
    return true;
  }

  bool set_active_interface(const String& id)
  {
    auto it = interface_id_to_index.find(id);
    if (it == interface_id_to_index.end()) return false;
    active.interface = it->second;
    return true;
  }

  bool set_active_responses(const String& id)
  {
    auto it = responses_id_to_index.find(id);
    if (it == responses_id_to_index.end()) return false;
    active.responses = it->second;
    return true;
  }
};

} // namespace Dakota

#endif // DAKOTA_IR_STATE_H
