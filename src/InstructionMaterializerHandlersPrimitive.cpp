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
#include <cstdint>
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

uint64_t enum_literal_to_u64(const Dakota::irgen::OpLiteral& lit,
                             const Dakota::irgen::IrValueType t)
{
  using Dakota::IRValue;
  using Dakota::irgen::IrValueType;

  auto as_u64 = [](const IRValue& v, const char* what) -> uint64_t {
    if (const auto* p = std::get_if<unsigned short>(&v)) return static_cast<uint64_t>(*p);
    if (const auto* p = std::get_if<short>(&v)) return static_cast<uint64_t>(*p);
    if (const auto* p = std::get_if<int>(&v)) return static_cast<uint64_t>(*p);
    if (const auto* p = std::get_if<size_t>(&v)) return static_cast<uint64_t>(*p);
    throw std::runtime_error(std::string("InstructionMaterializer::") + what +
                             " literal has non-integral variant value");
  };

  switch (t) {
  case IrValueType::UnsignedShort:
  case IrValueType::Short:
  case IrValueType::Int:
  case IrValueType::SizeT:
    return as_u64(lit.value, "enum handler");
  default:
    throw std::runtime_error(
      "InstructionMaterializer enum handler requires integral contract type");
  }
}

Dakota::IRValue enum_u64_to_irvalue(uint64_t v, Dakota::irgen::IrValueType t)
{
  using Dakota::IRValue;
  using Dakota::irgen::IrValueType;
  switch (t) {
  case IrValueType::UnsignedShort:
    return IRValue(static_cast<unsigned short>(v));
  case IrValueType::Short:
    return IRValue(static_cast<short>(v));
  case IrValueType::Int:
    return IRValue(static_cast<int>(v));
  case IrValueType::SizeT:
    return IRValue(static_cast<size_t>(v));
  default:
    throw std::runtime_error(
      "InstructionMaterializer enum handler requires integral contract type");
  }
}

uint64_t current_enum_value_or_zero(const Dakota::IRStore& store,
                                    const std::string& key,
                                    Dakota::irgen::IrValueType t)
{
  using Dakota::irgen::IrValueType;
  if (!store.contains(key))
    return 0;

  switch (t) {
  case IrValueType::UnsignedShort:
    return static_cast<uint64_t>(store.get<unsigned short>(key));
  case IrValueType::Short:
    return static_cast<uint64_t>(store.get<short>(key));
  case IrValueType::Int:
    return static_cast<uint64_t>(store.get<int>(key));
  case IrValueType::SizeT:
    return static_cast<uint64_t>(store.get<size_t>(key));
  default:
    throw std::runtime_error(
      "InstructionMaterializer enum handler requires integral contract type");
  }
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

void InstructionMaterializer::handle_uncertain_init_point_flag(
  const irgen::WriteOp& op,
  const irgen::KeyContract& contract,
  const HandlerContext& ctx)
{
  (void)contract;
  const auto& value = InstructionMaterializerUtils::required_path(
    ctx.block_json, ctx.current_path);
  if (!value.is_boolean()) {
    throw std::runtime_error(
      "InstructionMaterializer::handle_uncertain_init_point_flag expected boolean at '" +
      std::string(ctx.current_path) + "'");
  }
  if (value.get<bool>())
    ctx.store.set_value(op.target_local_ir_key, IRValue(true));
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
  (void)InstructionMaterializerUtils::required_path(ctx.block_json, ctx.current_path);
  const uint64_t lit = enum_literal_to_u64(op.literal, contract.ir_value_type);
  ctx.store.set_value(op.target_local_ir_key, enum_u64_to_irvalue(lit, contract.ir_value_type));
}

void InstructionMaterializer::handle_augment_enum(const irgen::WriteOp& op,
                                                  const irgen::KeyContract& contract,
                                                  const HandlerContext& ctx)
{
  (void)InstructionMaterializerUtils::required_path(ctx.block_json, ctx.current_path);
  const uint64_t base = current_enum_value_or_zero(
    ctx.store, op.target_local_ir_key, contract.ir_value_type);
  const uint64_t lit = enum_literal_to_u64(op.literal, contract.ir_value_type);
  const uint64_t out = (base | lit);
  ctx.store.set_value(op.target_local_ir_key, enum_u64_to_irvalue(out, contract.ir_value_type));
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

void InstructionMaterializer::handle_id_to_index_set(const irgen::WriteOp& op,
                                                     const irgen::KeyContract& contract,
                                                     const HandlerContext& ctx)
{
  (void)contract;
  const auto& value = InstructionMaterializerUtils::required_path(
    ctx.block_json, ctx.current_path);
  if (!value.is_array()) {
    throw std::runtime_error(
      "InstructionMaterializer::handle_id_to_index_set expected array at '" +
      std::string(ctx.current_path) + "'");
  }

  SizetSet out;
  for (size_t i = 0; i < value.size(); ++i) {
    if (!value[i].is_number_integer()) {
      throw std::runtime_error(
        "InstructionMaterializer::handle_id_to_index_set expected integer entries at '" +
        std::string(ctx.current_path) + "'");
    }
    const int id = value[i].get<int>();
    out.insert(static_cast<size_t>(id - 1));
  }

  ctx.store.set_value(op.target_local_ir_key, IRValue(std::move(out)));
}

void InstructionMaterializer::handle_int_set(const irgen::WriteOp& op,
                                             const irgen::KeyContract& contract,
                                             const HandlerContext& ctx)
{
  (void)contract;
  const auto& value = InstructionMaterializerUtils::required_path(
    ctx.block_json, ctx.current_path);
  if (!value.is_array()) {
    throw std::runtime_error(
      "InstructionMaterializer::handle_int_set expected array at '" +
      std::string(ctx.current_path) + "'");
  }

  IntSet out;
  for (size_t i = 0; i < value.size(); ++i) {
    if (!value[i].is_number_integer()) {
      throw std::runtime_error(
        "InstructionMaterializer::handle_int_set expected integer entries at '" +
        std::string(ctx.current_path) + "'");
    }
    out.insert(value[i].get<int>());
  }

  ctx.store.set_value(op.target_local_ir_key, IRValue(std::move(out)));
}

} // namespace Dakota
