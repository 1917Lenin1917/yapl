//
// Created by Максим Литвиненко on 26.08.2025.
//

#include "yapl/ByteCodeVM.hpp"

#include "yapl/values/CodeObjectValue.hpp"
#include "yapl/values/UndefinedValue.hpp"
#include "yapl/values/FunctionValue.hpp"

namespace yapl {

void ByteCodeVM::Run(CodeObject &code)
{
  std::size_t idx = 0;

  while (true)
  {
    const auto op = code.OpCodes[idx++];

    switch (op)
    {
      case JMP:
      {
        const auto rel = static_cast<int>(code.OpCodes[idx++]);
        idx += rel;
        break;
      }

      case JMP_IF_FALSE:
      {
        const auto val = m_Stack.top();
        m_Stack.pop();

        const auto rel = code.OpCodes[idx++];
        if (!val->IsTruthy())
        {
          idx += rel;
        }
        break;
      }

      case LOAD_CONST:
      {
        const auto const_idx = code.OpCodes[idx++];
        m_Stack.push(code.Constants[const_idx]);
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
        const auto local_idx = code.OpCodes[idx++];
        auto& var = code.Locals[local_idx];
        auto value = m_Stack.top();
        m_Stack.pop();

        var->is_tdz = false;
        var->value = value;
        break;
      }

      case DEINIT_VAR:
      {
        const auto local_idx = code.OpCodes[idx++];
        auto& var = code.Locals[local_idx];

        var->is_tdz = true;
        var->value = nullptr;
        break;
      }

      case BINARY_OP:
      {
        HandleBinaryOp(static_cast<BinaryOp>(code.OpCodes[idx++]));
        break;
      }

      case LOAD_NAME:
      {
        // const auto local_idx = code.OpCodes[idx++];
        // m_Stack.push(code.Locals[local_idx]->value);
        const auto name_idx = code.OpCodes[idx++];
        const auto& name = code.Names[name_idx];
        bool found = false;
        for (const auto& local : code.Locals)
        {
          if (name == local->name)
          {
            m_Stack.push(local->value);
            found = true;
            break;
          }
        }
        if (!found) m_Stack.push(mk_undefined());

        break;
      }

      case STORE_NAME:
      {
        const auto local_idx = code.OpCodes[idx++];
        const auto var = code.Locals[local_idx];
        const auto value = m_Stack.top();
        m_Stack.pop();

        var->value = value;
        break;
      }

      case MAKE_FUNC:
      {
        const auto code_object_value = m_Stack.top();
        m_Stack.pop();

        auto fn = mk_func("TODO", static_cast<CodeObjectValue*>(code_object_value.get())->code_object);
        m_Stack.push(fn);
        break;
      }

      case CALL:
      {
        // todo: pop args
        const auto fn_obj = m_Stack.top();
        m_Stack.pop();

        fn_obj->tp->nb_call(*this, fn_obj);

        break;
      }
      case RETURN:
      {
        // todo: maybe do something else? :)
        return;
      }

      case HALT:
      {
        return;
      }

      default:
        throw std::runtime_error("Unhandled Op");
    } // switch
  } // while
}

// Keep the old Run() for running the VM's current m_CodeObject
void ByteCodeVM::Run()
{
  Run(m_CodeObject);
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
