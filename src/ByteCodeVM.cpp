//
// Created by Максим Литвиненко on 26.08.2025.
//

#include "yapl/ByteCodeVM.hpp"
#include "yapl/values/UndefinedValue.hpp"

namespace yapl {

void ByteCodeVM::Run()
{
  while (true)
  {
    switch (m_CodeObject.OpCodes[m_Idx++])
    {
      case JMP:
      {
        const auto idx = static_cast<int>(m_CodeObject.OpCodes[m_Idx++]);
        m_Idx += idx;
        break;
      }
      case JMP_IF_FALSE:
      {
        const auto val = m_Stack.top();
        m_Stack.pop();

        const auto idx = m_CodeObject.OpCodes[m_Idx++];
        if (!val->IsTruthy())
        {
          m_Idx += idx;
        }

        break;
      }
      case LOAD_CONST:
      {
        const auto idx = m_CodeObject.OpCodes[m_Idx++];
        m_Stack.push(m_CodeObject.Constants[idx]);
        break;
      }
      case LOAD_UNDEF:
      {
        // TODO: change to load from globals !
        m_Stack.push(mk_undefined());
        break;
      }
      case INIT_VAR:
      {
        const auto idx = m_CodeObject.OpCodes[m_Idx++];
        auto& var = m_CodeObject.Locals[idx];
        auto value = m_Stack.top();
        m_Stack.pop();

        var->is_tdz = false;
        var->value = value;

        break;
      }
      case DEINIT_VAR:
      {
        const auto idx = m_CodeObject.OpCodes[m_Idx++];
        auto& var = m_CodeObject.Locals[idx];

        var->is_tdz = true;
        var->value = nullptr;
        break;
      }
      case BINARY_OP:
      {
        HandleBinaryOp(static_cast<BinaryOp>(m_CodeObject.OpCodes[m_Idx++]));
        break;
      }
      case LOAD_NAME: {
        const auto idx = m_CodeObject.OpCodes[m_Idx++];
        m_Stack.push(m_CodeObject.Locals[idx]->value);
        break;
      }
      case STORE_NAME: {
        const auto idx = m_CodeObject.OpCodes[m_Idx++];
        const auto var = m_CodeObject.Locals[idx];
        const auto value = m_Stack.top();
        m_Stack.pop();

        var->value = value;
        break;
      }
      case HALT: {
        return;
      }
      default: throw std::runtime_error("Unhandled Op");
    }
  }
}


void ByteCodeVM::HandleBinaryOp(const BinaryOp compare_type)
{
    const auto rhs = m_Stack.top();
    m_Stack.pop();
    const auto lhs = m_Stack.top();
    m_Stack.pop();

    binop_fn op, rop;

    switch (compare_type) {
      case ADD:
        op  = lhs->tp->nb_add;
        rop = rhs->tp->nb_add;
        break;

      case MUL:
        op  = lhs->tp->nb_mul;
        rop = rhs->tp->nb_mul;
        break;

      case SUB:
        op  = lhs->tp->nb_sub;
        rop = rhs->tp->nb_sub;
        break;

      case DIV:
        op  = lhs->tp->nb_div;
        rop = rhs->tp->nb_div;
        break;

      case MOD:
        op  = lhs->tp->nb_mod;
        rop = rhs->tp->nb_mod;
        break;

      case LT:
        op  = lhs->tp->nb_lt;
        rop = rhs->tp->nb_lt;
        break;

      case GT:
        op  = lhs->tp->nb_gt;
        rop = rhs->tp->nb_gt;
        break;

      case LQ:
        op  = lhs->tp->nb_le;
        rop = rhs->tp->nb_le;
        break;

      case GQ:
        op  = lhs->tp->nb_ge;
        rop = rhs->tp->nb_ge;
        break;

      case EQ:
        op  = lhs->tp->nb_eq;
        rop = rhs->tp->nb_eq;
        break;

      case NEQ:
        op  = lhs->tp->nb_nq;
        rop = rhs->tp->nb_nq;
        break;

      case AND:
        op  = lhs->tp->nb_and;
        rop = rhs->tp->nb_and;
        break;

      case OR:
        op  = lhs->tp->nb_or;
        rop = rhs->tp->nb_or;
        break;

      default:
        // handle invalid compare_type if needed
        break;
    }

    if (op) {
      VPtr r = op(lhs, rhs);
      if (r != NotImplemented) { m_Stack.push(r); return; }
    }
    if (rop) {            // reverse call
      VPtr r = rop(rhs, lhs);
      if (r != NotImplemented) { m_Stack.push(r); return; }
    }
}

}
