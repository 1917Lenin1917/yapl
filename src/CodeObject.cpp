//
// Created by lenin on 09.12.2025.
//

#include "yapl/CodeObject.hpp"

#include "yapl/Serialization.hpp"
#include "yapl/values/Value.hpp"

namespace yapl {
std::vector<std::byte> Parameter::Serialize() const
{
    std::vector<std::byte> buffer;

    const auto nameLength = name.size();

    appendUint16(buffer, nameLength);
    appendStringBytes(buffer, name);
    appendByte(buffer, static_cast<std::byte>(kind));
    appendByte(buffer, static_cast<std::byte>(has_default));
    appendUint16(buffer, default_const_index);
    appendUint16(buffer, local_index);

    return buffer;
}

Parameter Parameter::Deserialize(const std::vector<std::byte> &bytes, std::size_t &offset)
{
  auto name_len = readUint16(bytes, offset);
  auto name = readStringBytes(bytes, offset, name_len);

  auto kind = readByte(bytes, offset);
  auto has_default = readByte(bytes, offset);

  auto default_const_index = readUint16(bytes, offset);
  auto local_index = readUint16(bytes, offset);

  return {
  .name = name,
  .kind = static_cast<ParamKind>(kind),
  .has_default = static_cast<bool>(has_default),
  .default_const_index = default_const_index,
  .local_index = local_index
  };
}

std::vector<std::byte> Export::Serialize() const
{
    std::vector<std::byte> buffer;

    const auto nameLength = name.size();

    appendUint16(buffer, nameLength);
    appendStringBytes(buffer, name);
    appendByte(buffer, static_cast<std::byte>(kind));
    appendUint32(buffer, index);

    return buffer;
}

Export Export::Deserialize(const std::vector<std::byte> &bytes, std::size_t &offset)
{
  auto module_name_len = readUint16(bytes, offset);
  auto module_name = readStringBytes(bytes, offset, module_name_len);

  auto kind = readByte(bytes, offset);
  auto index = readUint32(bytes, offset);

  return {
    .kind = static_cast<ExportKind>(kind),
    .index = static_cast<std::size_t>(index),
    .name = module_name,
  };
}

std::vector<std::byte> CodeObject::Serialize() const
{
    std::vector<std::byte> buffer;

    // appendByte(buffer, static_cast<std::byte>(CODE_OBJECT_VERSION));
    const auto nameLength = name.size();

    appendUint16(buffer, nameLength);
    appendStringBytes(buffer, name);

    appendUint16(buffer, names.size());
    for (const std::string& currentName : names)
    {
      const auto currentNameSize = currentName.size();
      appendUint16(buffer, currentNameSize);
      appendStringBytes(buffer, currentName);
    }

    appendUint16(buffer, locals.size());
    for (const std::string& currentLocal : locals)
    {
      const auto currentLocalSize = currentLocal.size();
      appendUint16(buffer, currentLocalSize);
      appendStringBytes(buffer, currentLocal);
    }

    appendUint16(buffer, params.size());
    for (const auto& currentParam : params)
    {
      auto serializedParam = currentParam.Serialize();
      buffer.insert(buffer.end(), serializedParam.begin(), serializedParam.end());
    }

    appendUint16(buffer, constants.size());
    for (const auto& currentConst : constants)
    {
      auto serializedParam = currentConst->Serialize();
      buffer.insert(buffer.end(), serializedParam.begin(), serializedParam.end());
    }

    appendUint16(buffer, op_codes.size());
    for (const auto& currentOpCode : op_codes)
    {
      appendUint32(buffer, currentOpCode);
    }

    appendUint16(buffer, exports.size());
    for (const auto& currentExport : exports)
    {
      auto serializedExport = currentExport.Serialize();
      buffer.insert(buffer.end(), serializedExport.begin(), serializedExport.end());
    }

    return buffer;
}

CodeObject CodeObject::Deserialize(const std::vector<std::byte> &bytes, std::size_t& offset)
{
  auto module_name_len = readUint16(bytes, offset);
  auto module_name = readStringBytes(bytes, offset, module_name_len);

  auto names_len = readUint16(bytes, offset);
  std::vector<std::string> names;
  names.reserve(names_len);

  for (std::size_t i = 0; i < names_len; i++)
  {
    auto name_len = readUint16(bytes, offset);
    names.push_back(readStringBytes(bytes, offset, name_len));
  }

  auto locals_len = readUint16(bytes, offset);
  std::vector<std::string> locals;
  locals.reserve(locals_len);

  for (std::size_t i = 0; i < locals_len; i++)
  {
    auto local_len = readUint16(bytes, offset);
    locals.push_back(readStringBytes(bytes, offset, local_len));
  }

  auto params_len = readUint16(bytes, offset);
  std::vector<Parameter> params;
  params.reserve(params_len);

  for (std::size_t i = 0; i < params_len; i++)
  {
    params.push_back(Parameter::Deserialize(bytes, offset));
  }

  auto consts_len = readUint16(bytes, offset);
  std::vector<VPtr> constants;
  constants.reserve(consts_len);

  for (std::size_t i = 0; i < consts_len; i++)
  {
    constants.push_back(readValue(bytes, offset));
  }

  auto op_codes_len = readUint16(bytes, offset);
  std::vector<OpCode> op_codes;
  op_codes.reserve(op_codes_len);

  for (std::size_t i = 0; i < op_codes_len; i++)
  {
    op_codes.push_back(static_cast<OpCode>(readUint32(bytes, offset)));
  }

  return {
    .op_codes  = std::move(op_codes),
    .name      = module_name,
    .constants = std::move(constants),
    .locals    = std::move(locals),
    .names     = std::move(names),
    .params    = std::move(params)
  };
}


void print_code_object(const CodeObject &code_object, std::ostream &output)
{
  output << "CodeObject \"" << code_object.name << "\"\n";
  output << "  args: " << code_object.params.size() << "\n";
  output << "  constants: " << code_object.constants.size() << "\n";
  output << "  locals: " << code_object.locals.size() << "\n";
  output << "  names: " << code_object.names.size() << "\n";
  output << "  bytecode size: " << code_object.op_codes.size() << "\n\n";

  std::size_t instruction_pointer = 0;
  const std::size_t opcodes_size = code_object.op_codes.size();

  auto read_operand = [&](std::size_t& ip, std::size_t& operand) -> bool {
    if (ip >= opcodes_size) {
      return false;
    }
    operand = static_cast<std::size_t>(code_object.op_codes[ip]);
    ++ip;
    return true;
  };

  while (instruction_pointer < opcodes_size) {
    const std::size_t opcode_index = instruction_pointer;
    const OpCode opcode = code_object.op_codes[instruction_pointer];
    ++instruction_pointer;

    output << opcode_index << ":\t" << opcode_to_string(opcode);

    switch (opcode) {
      case OpCode::LOAD_CONST:
      {
        std::size_t operand = 0;
        if (read_operand(instruction_pointer, operand)) {
          output << " " << operand;
          if (operand < code_object.constants.size() && code_object.constants[operand]) {
            output << " (";
            output << code_object.constants[operand]->print();
            output << ")";
          }
        } else {
          output << " <missing operand>";
        }
        output << "\n";
        break;
      }

      case OpCode::LOAD_LOCAL:
      case OpCode::STORE_LOCAL:
      case OpCode::DEINIT_VAR:
      case OpCode::INIT_VAR: {
        std::size_t operand = 0;
        if (read_operand(instruction_pointer, operand)) {
          output << " " << operand;
          if (operand < code_object.locals.size()) {
            output << " (" << code_object.locals[operand] << ")";
          }
        } else {
          output << " <missing operand>";
        }
        output << "\n";
        break;
      }

      case OpCode::MAKE_ARR:
      case OpCode::CALL: {
        std::size_t operand = 0;
        if (read_operand(instruction_pointer, operand)) {
          output << " " << operand;
        } else {
          output << " <missing operand>";
        }
        output << "\n";
        break;
      }
      case OpCode::KW_CALL: {
        std::size_t pos_arg_amount = 0;
        if (read_operand(instruction_pointer, pos_arg_amount)) {
          output << " " << pos_arg_amount << " ";
        } else {
          output << " <missing operand> ";
        }
        std::size_t kw_arg_amount = 0;
        if (read_operand(instruction_pointer, kw_arg_amount)) {
          output << " " << kw_arg_amount;
        } else {
          output << " <missing operand>";
        }
        output << "\n";
        break;
      }

      case OpCode::MAKE_TYPE:
      {
        std::size_t operand = 0;
        if (read_operand(instruction_pointer, operand)) {
          output << " " << operand;
          if (operand < code_object.names.size()) {
            output << " (" << code_object.names[operand] << ")";
          }
        } else {
          output << " <missing operand>";
        }
        std::size_t method_amount = 0;
        if (read_operand(instruction_pointer, method_amount)) {
          output << " " << method_amount;
        } else {
          output << " <missing operand>";
        }
        output << "\n";
        break;
      }

      case OpCode::CALL_METHOD:
      {
        std::size_t operand = 0;
        if (read_operand(instruction_pointer, operand)) {
          output << " " << operand;
          if (operand < code_object.names.size()) {
            output << " (" << code_object.names[operand] << ")";
          }
        } else {
          output << " <missing operand>";
        }
        std::size_t method_amount = 0;
        if (read_operand(instruction_pointer, method_amount)) {
          output << " " << method_amount;
        } else {
          output << " <missing operand>";
        }
        output << "\n";
        break;
      }

      case OpCode::KW_CALL_METHOD:
      {
        std::size_t operand = 0;
        if (read_operand(instruction_pointer, operand)) {
          output << " " << operand;
          if (operand < code_object.names.size()) {
            output << " (" << code_object.names[operand] << ")";
          }
        } else {
          output << " <missing operand>";
        }
        std::size_t method_amount = 0;
        if (read_operand(instruction_pointer, method_amount)) {
          output << " " << method_amount;
        } else {
          output << " <missing operand>";
        }
        std::size_t method_amount_2 = 0;
        if (read_operand(instruction_pointer, method_amount_2)) {
          output << " " << method_amount_2;
        } else {
          output << " <missing operand>";
        }
        output << "\n";
        break;
      }

      case OpCode::IMPORT_NAME:
      case OpCode::LOAD_NAME:
      case OpCode::STORE_NAME:
      case OpCode::SET_PROPERTY:
      case OpCode::GET_PROPERTY:
      {
        std::size_t operand = 0;
        if (read_operand(instruction_pointer, operand)) {
          output << " " << operand;
          if (operand < code_object.names.size()) {
            output << " (" << code_object.names[operand] << ")";
          }
        } else {
          output << " <missing operand>";
        }
        output << "\n";
        break;
      }

      case OpCode::BINARY_OP: {
        std::size_t operand = 0;
        if (read_operand(instruction_pointer, operand)) {
          const BinaryOp binary_op = static_cast<BinaryOp>(operand);
          output << " " << binary_op_to_string(binary_op);
        } else {
          output << " <missing operand>";
        }
        output << "\n";
        break;
      }

      case OpCode::JMP:
      case OpCode::JMP_IF_FALSE: {
        std::size_t operand = 0;
        if (read_operand(instruction_pointer, operand)) {
          output << " " << operand;
        } else {
          output << " <missing operand>";
        }
        output << "\n";
        break;
      }

      case OpCode::LOAD_MODULE:
      case OpCode::POP:
      case OpCode::NOP:
      case OpCode::RETURN:
      case OpCode::HALT:
      case OpCode::LOAD_UNDEF:
      case OpCode::MAKE_FUNC: {
        output << "\n";
        break;
      }
    }
  }
}
}

// current_object.op_codes.push_back(MAKE_TYPE);
// current_object.op_codes.push_back(static_cast<OpCode>(name_index));
// current_object.op_codes.push_back(static_cast<OpCode>(node.member_functions.size()));
