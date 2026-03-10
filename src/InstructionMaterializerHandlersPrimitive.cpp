/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "InstructionMaterializer.hpp"
#include "InstructionMaterializerUtils.hpp"
#include "JSONUtils.hpp"

#include <nlohmann/json.hpp>

#include <cctype>
#include <stdexcept>
#include <string>

namespace {

using json = nlohmann::json;

Dakota::IRValue convert_direct_value(const json& value, dakota::irgen::IrValueType t)
{
  using namespace Dakota;

  switch (t) {
  case irgen::IrValueType::String: return IRValue(value.get<String>());
  case irgen::IrValueType::Bool: return IRValue(value.get<bool>());
  case irgen::IrValueType::Int: return IRValue(value.get<int>());
  case irgen::IrValueType::Short: return IRValue(value.get<short>());
  case irgen::IrValueType::SizeT: return IRValue(value.get<size_t>());
  case irgen::IrValueType::UnsignedShort: return IRValue(value.get<unsigned short>());
  case irgen::IrValueType::Real: return IRValue(value.get<Real>());
  case irgen::IrValueType::StringArray: return IRValue(value.get<StringArray>());
  case irgen::IrValueType::String2DArray: return IRValue(value.get<String2DArray>());
  case irgen::IrValueType::IntVector: return IRValue(value.get<JSONIntVector>().value);
  case irgen::IrValueType::RealVector: return IRValue(value.get<JSONRealVector>().value);
  case irgen::IrValueType::BitArray: return IRValue(value.get<JSONBitArray>().value);
  case irgen::IrValueType::SizetArray: return IRValue(value.get<SizetArray>());
  case irgen::IrValueType::UShortArray: return IRValue(value.get<UShortArray>());
  case irgen::IrValueType::RealSymMatrix: return IRValue(value.get<JSONRealSymMatrix>().value);
  case irgen::IrValueType::IntSet: return IRValue(value.get<IntSet>());
  case irgen::IrValueType::SizetSet: return IRValue(value.get<SizetSet>());
  case irgen::IrValueType::IntSetArray: return IRValue(value.get<IntSetArray>());
  case irgen::IrValueType::StringSetArray: return IRValue(value.get<StringSetArray>());
  case irgen::IrValueType::RealSetArray: return IRValue(value.get<RealSetArray>());
  case irgen::IrValueType::IntRealMapArray: return IRValue(value.get<IntRealMapArray>());
  case irgen::IrValueType::StringRealMapArray: return IRValue(value.get<StringRealMapArray>());
  case irgen::IrValueType::RealRealMapArray: return IRValue(value.get<RealRealMapArray>());
  case irgen::IrValueType::RealRealPairRealMapArray:
    return IRValue(value.get<RealRealPairRealMapArray>());
  case irgen::IrValueType::IntIntPairRealMapArray:
    return IRValue(value.get<IntIntPairRealMapArray>());
  case irgen::IrValueType::RealMatrixArray:
    return IRValue(value.get<JSONRealMatrixArray>().value);
  case irgen::IrValueType::RealVectorArray:
    return IRValue(value.get<JSONRealVectorArray>().value);
  case irgen::IrValueType::UnspecifiedType:
    return IRValue(value);
  }

  return IRValue(value);
}

} // namespace

namespace Dakota {

void InstructionMaterializer::handle_direct_value(const irgen::WriteOp& op,
                                                  const irgen::KeyContract& contract,
                                                  const HandlerContext& ctx)
{
  const auto& value = InstructionMaterializerUtils::required_path(
    ctx.block_json, ctx.current_path);
  ctx.store.set_value(op.target_local_ir_key,
                      convert_direct_value(value, contract.ir_value_type));
}

void InstructionMaterializer::handle_literal_assign(const irgen::WriteOp& op,
                                                    const irgen::KeyContract& contract,
                                                    const HandlerContext& ctx)
{
  (void)contract;
  ctx.store.set_value(op.target_local_ir_key, op.literal.value);
}

void InstructionMaterializer::handle_presence_bool(const irgen::WriteOp& op,
                                                   const irgen::KeyContract& contract,
                                                   const HandlerContext& ctx)
{
  (void)contract;
  const auto& value = InstructionMaterializerUtils::required_path(
    ctx.block_json, ctx.current_path);
  if (value.is_object()) {
    ctx.store.set_value(op.target_local_ir_key, IRValue(true));
    return;
  }
  if (!value.is_boolean()) {
    throw std::runtime_error(
      "InstructionMaterializer::handle_presence_bool expected object or boolean at '" +
      std::string(ctx.current_path) + "'");
  }
  ctx.store.set_value(op.target_local_ir_key, IRValue(value.get<bool>()));
}

void InstructionMaterializer::handle_presence_literal(const irgen::WriteOp& op,
                                                      const irgen::KeyContract& contract,
                                                      const HandlerContext& ctx)
{
  (void)contract;
  (void)InstructionMaterializerUtils::required_path(ctx.block_json, ctx.current_path);
  ctx.store.set_value(op.target_local_ir_key, op.literal.value);
}

void InstructionMaterializer::handle_presence_enum(const irgen::WriteOp& op,
                                                   const irgen::KeyContract& contract,
                                                   const HandlerContext& ctx)
{
  (void)op;
  (void)contract;
  (void)ctx;
  throw std::runtime_error(
    "InstructionMaterializer::handle_presence_enum disabled: instruction literal typed as String");
}

void InstructionMaterializer::handle_augment_enum(const irgen::WriteOp& op,
                                                  const irgen::KeyContract& contract,
                                                  const HandlerContext& ctx)
{
  (void)op;
  (void)contract;
  (void)ctx;
  throw std::runtime_error(
    "InstructionMaterializer::handle_augment_enum disabled: instruction literal typed as String");
}

void InstructionMaterializer::handle_categorical(const irgen::WriteOp& op,
                                                 const irgen::KeyContract& contract,
                                                 const HandlerContext& ctx)
{
  (void)contract;
  const auto& value = InstructionMaterializerUtils::required_path(
    ctx.block_json, ctx.current_path);
  if (!value.is_array()) {
    throw std::runtime_error(
      "InstructionMaterializer::handle_categorical expected array at '" +
      std::string(ctx.current_path) + "'");
  }

  BitArray bits(value.size());
  for (size_t i = 0; i < value.size(); ++i) {
    if (!value[i].is_string()) {
      throw std::runtime_error(
        "InstructionMaterializer::handle_categorical expected string entries at '" +
        std::string(ctx.current_path) + "'");
    }
    const std::string s = value[i].get<std::string>();
    if (!s.empty()) {
      const char c = static_cast<char>(std::tolower(static_cast<unsigned char>(s[0])));
      bits[i] = (c == 't' || c == 'y');
    }
    else {
      bits[i] = false;
    }
  }
  ctx.store.set_value(op.target_local_ir_key, IRValue(std::move(bits)));
}

} // namespace Dakota
