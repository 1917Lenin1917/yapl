//
// Created by Максим Литвиненко on 25.08.2025.
//

#pragma once
#include <string_view>
#include <cstdint>

namespace yapl {

enum OpCode : std::uint32_t {
  NOP,
  RETURN,
  HALT,
  POP, // Pops from the stack

  LOAD_CONST,
  LOAD_UNDEF, // pushes undefined to stack

  MAKE_ARR,
  MAKE_TYPE,
  MAKE_FUNC,

  KW_CALL, // call when we have key arguments like op="+"
  CALL,
  CALL_METHOD,
  KW_CALL_METHOD,

  GET_PROPERTY,
  SET_PROPERTY,

  INIT_VAR,   // sets `is_tdz` to false and assigns value from stack top
  DEINIT_VAR, // sets `is_tdz` to true

  LOAD_LOCAL,
  STORE_LOCAL,

  LOAD_NAME,
  STORE_NAME,

  LOAD_MODULE,
  IMPORT_NAME,

  UNARY_OP,
  BINARY_OP,

  JMP,
  JMP_IF_FALSE,
};

enum UnaryOp {
  POS,  // +
  NEG, // -
  NOT,   // !
};

enum BinaryOp {
  ADD,    // +
  SUB,    // -
  MUL,    // *
  MOD,    // %
  DIV,    // /
  EQ,     // ==
  NEQ,    // !=
  LT,     // <
  LQ,     // <=
  GT,     // >
  GQ,     // >=
  OR,     // or
  AND,    // and
};

inline std::string_view opcode_to_string(const OpCode opcode) {
  switch (opcode) {
    case OpCode::NOP:            return "NOP";
    case OpCode::RETURN:         return "RETURN";
    case OpCode::HALT:           return "HALT";
    case OpCode::LOAD_CONST:     return "LOAD_CONST";
    case OpCode::LOAD_UNDEF:     return "LOAD_UNDEF";
    case OpCode::MAKE_FUNC:      return "MAKE_FUNC";
    case OpCode::CALL:           return "CALL";
    case OpCode::KW_CALL:        return "KW_CALL";
    case OpCode::CALL_METHOD:    return "CALL_METHOD";
    case OpCode::KW_CALL_METHOD: return "KW_CALL_METHOD";
    case OpCode::INIT_VAR:       return "INIT_VAR";
    case OpCode::DEINIT_VAR:     return "DEINIT_VAR";
    case OpCode::LOAD_LOCAL:     return "LOAD_LOCAL";
    case OpCode::STORE_LOCAL:    return "STORE_LOCAL";
    case OpCode::LOAD_NAME:      return "LOAD_NAME";
    case OpCode::STORE_NAME:     return "STORE_NAME";
    case OpCode::BINARY_OP:      return "BINARY_OP";
    case OpCode::JMP:            return "JMP";
    case OpCode::JMP_IF_FALSE:   return "JMP_IF_FALSE";
    case OpCode::MAKE_ARR:       return "MAKE_ARR";
    case OpCode::GET_PROPERTY:   return "GET_PROPERTY";
    case OpCode::SET_PROPERTY:   return "SET_PROPERTY";
    case OpCode::UNARY_OP:       return "UNARY_OP";
    case OpCode::MAKE_TYPE:      return "MAKE_TYPE";
    case OpCode::POP:            return "POP";
    case OpCode::IMPORT_NAME:    return "IMPORT_NAME";
    case OpCode::LOAD_MODULE:    return "LOAD_MODULE";
  }
  return "UNKNOWN";
}

inline std::string_view binary_op_to_string(BinaryOp binary_op) {
  switch (binary_op) {
    case BinaryOp::ADD: return "ADD";
    case BinaryOp::SUB: return "SUB";
    case BinaryOp::MUL: return "MUL";
    case BinaryOp::MOD: return "MOD";
    case BinaryOp::DIV: return "DIV";
    case BinaryOp::EQ:  return "EQ";
    case BinaryOp::NEQ: return "NEQ";
    case BinaryOp::LT:  return "LT";
    case BinaryOp::LQ:  return "LQ";
    case BinaryOp::GT:  return "GT";
    case BinaryOp::GQ:  return "GQ";
    case BinaryOp::OR:  return "OR";
    case BinaryOp::AND: return "AND";
  }
  return "UNKNOWN_BINOP";
}


}
