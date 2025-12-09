//
// Created by lenin on 09.12.2025.
//

#include "yapl/CodeObject.hpp"
#include "yapl/values/Value.hpp"

namespace yapl {

void print_code_object(const CodeObject &code_object, std::ostream &output)
{
  output << "CodeObject \"" << code_object.name << "\"\n";
  output << "  args: " << code_object.arg_count << "\n";
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
case OpCode::LOAD_CONST: {
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

      case OpCode::LOAD_NAME:
      case OpCode::STORE_NAME: {
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
