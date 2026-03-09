// Stable table metadata types. Keep this header minimal and non-derived.
#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>
#include "generated_ir_types.hpp"

namespace dakota::irgen {

enum class BlockType { Environment, Method, Model, Variables, Interface, Responses };

enum class DefaultSource { Policy, Schema, Override };

struct KeyContract {
  IrValueType ir_value_type;
  DefaultSource default_source;
  IRValue default_value;
};

struct WriteOp {
  OpKind op_kind;
  std::string target_local_pdb_key;
  nlohmann::json payload;
};

struct BlockTables {
  const std::unordered_map<std::string, KeyContract>& contracts;
  const std::unordered_map<std::string, std::vector<WriteOp>>& instructions;
};

namespace environment {
const std::unordered_map<std::string, KeyContract>& contracts();
const std::unordered_map<std::string, std::vector<WriteOp>>& instructions();
}
namespace method {
const std::unordered_map<std::string, KeyContract>& contracts();
const std::unordered_map<std::string, std::vector<WriteOp>>& instructions();
}
namespace model {
const std::unordered_map<std::string, KeyContract>& contracts();
const std::unordered_map<std::string, std::vector<WriteOp>>& instructions();
}
namespace variables {
const std::unordered_map<std::string, KeyContract>& contracts();
const std::unordered_map<std::string, std::vector<WriteOp>>& instructions();
}
namespace interface {
const std::unordered_map<std::string, KeyContract>& contracts();
const std::unordered_map<std::string, std::vector<WriteOp>>& instructions();
}
namespace responses {
const std::unordered_map<std::string, KeyContract>& contracts();
const std::unordered_map<std::string, std::vector<WriteOp>>& instructions();
}

const BlockTables& tables_for_block(BlockType block);

} // namespace dakota::irgen
