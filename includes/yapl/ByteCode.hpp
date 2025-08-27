//
// Created by Максим Литвиненко on 25.08.2025.
//

#pragma once

namespace yapl {

enum OpCode {
  NOP,
  RETURN,
  HALT,

  LOAD_CONST,
  LOAD_NAME,
  STORE_NAME,

  BINARY_OP,

  JMP,
  JMP_IF_FALSE,
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

}
