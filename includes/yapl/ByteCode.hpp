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
  LOAD_UNDEF, // pushes undefined to stack

  INIT_VAR,   // sets `is_tdz` to false and assigns value from stack top
  DEINIT_VAR, // sets `is_tdz` to true

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
