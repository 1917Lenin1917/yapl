//
// Created by Максим Литвиненко on 26.08.2025.
//

#include "yapl/ByteCodeVM.hpp"

#include <bits/ranges_algo.h>
#include <yapl/values/ArrayValue.hpp>

#include "yapl/values/CodeObjectValue.hpp"
#include "yapl/values/UndefinedValue.hpp"
#include "yapl/values/FunctionValue.hpp"
#include "yapl/values/IntegerValue.hpp"

namespace yapl {

void ByteCodeVM::Run(CodeObject &code)
{
  std::size_t idx = 0;

  while (true)
  {
    if (idx == code.op_codes.size()) return;
    const auto op = code.op_codes[idx++];

    switch (op)
    {
      case JMP:
      {
        const auto rel = static_cast<int>(code.op_codes[idx++]);
        idx += rel;
        break;
      }
      case JMP_IF_FALSE:
      {
        const auto val = m_Stack.top();
        m_Stack.pop();

        const auto rel = code.op_codes[idx++];
        if (!val->IsTruthy())
        {
          idx += rel;
        }
        break;
      }
      case LOAD_CONST:
      {
        const auto const_idx = code.op_codes[idx++];
        m_Stack.push(code.constants[const_idx]);
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
        const auto& frame = m_FrameStack.back();

        const auto local_idx = code.op_codes[idx++];
        const auto& var = frame.locals[local_idx];
        const auto value = m_Stack.top();
        m_Stack.pop();

        var->is_tdz = false;
        var->value = value;
        break;
      }
      case DEINIT_VAR:
      {
        // FIXME: handle this before return and uncomment
        auto& frame = m_FrameStack.back();

        const auto local_idx = code.op_codes[idx++];
        auto& var = frame.locals[local_idx];

        var->is_tdz = true;
        var->value = nullptr;
        break;
      }
      case UNARY_OP:
      {
        HandleUnaryOp(static_cast<UnaryOp>(code.op_codes[idx++]));
        break;
      }
      case BINARY_OP:
      {
        HandleBinaryOp(static_cast<BinaryOp>(code.op_codes[idx++]));
        break;
      }
      case LOAD_NAME:
      {
        // At this moment, just look up globals
        const auto name_idx = code.op_codes[idx++];
        const auto& name = code.names[name_idx];
        const auto& var = m_Globals.contains(name) ? m_Globals.at(name) : nullptr;

        if (var) m_Stack.push(var->value);
        else m_Stack.push(mk_undefined());

        break;
      }
      case LOAD_LOCAL:
      {
        auto& frame = m_FrameStack.back();

        const auto locals_index = code.op_codes[idx++];
        const auto var = frame.locals[locals_index];
        m_Stack.push(var->value);
        break;
      }
      case STORE_LOCAL:
      {
        auto& frame = m_FrameStack.back();

        const auto locals_index = code.op_codes[idx++];
        const auto var = frame.locals[locals_index];
        const auto value = m_Stack.top();
        m_Stack.pop();

        var->value = value;
        var->type = value->type;
        break;

      }
      case STORE_NAME:
      {
        const auto names_index = code.op_codes[idx++];
        const auto name = code.names[names_index];
        const auto& var = m_Globals.contains(name) ? m_Globals.at(name) : nullptr;

        const auto value = m_Stack.top();
        m_Stack.pop();

        if (!var)
          throw std::runtime_error(std::format("Unknown variable {}", name));

        var->value = value;
        break;
      }
      case MAKE_FUNC:
      {
        const auto code_object_value = m_Stack.top();
        const auto casted = static_cast<CodeObjectValue*>(code_object_value.get());
        const auto name = casted->code_object->name;
        m_Stack.pop();

        auto fn = mk_func(name, static_cast<CodeObjectValue*>(code_object_value.get())->code_object);
        m_Globals[name] = std::make_shared<Variable>(true, VALUE_TYPE::FUNCTION, fn, "__main__", name, false);
        break;
      }
      case KW_CALL:
      {
        const int positional_arg_count = code.op_codes[idx++];
        const int keyword_arg_count = code.op_codes[idx++];

        const auto function_object = m_Stack.top();
        m_Stack.pop();

        m_FrameStack.push_back({});

        std::unordered_map<std::string, VPtr> keyword_arguments;
        keyword_arguments.reserve(keyword_arg_count);

        for (int i = 0; i < keyword_arg_count; i++)
        {
          const auto keyword_name_value = m_Stack.top();
          m_Stack.pop();

          const auto keyword_value = m_Stack.top();
          m_Stack.pop();

          const auto keyword_name = static_cast<StringValue*>(keyword_name_value.get())->value;
          keyword_arguments.emplace(keyword_name, keyword_value);
        }

        std::vector<VPtr> positional_arguments;
        positional_arguments.reserve(positional_arg_count);
        for (int i = 0; i < positional_arg_count; i++)
        {
          positional_arguments.push_back(m_Stack.top());
          m_Stack.pop();
        }
        std::ranges::reverse(positional_arguments);


        if (function_object->tp == BuiltinFunctionTypeObject)
        {
          auto dict = mk_dict();
          for (const auto& [key, value] : keyword_arguments)
          {
            dict->OperatorIndexSet(mk_str(key), value);
          }

          m_Stack.push(mk_arr(std::move(positional_arguments)));
          m_Stack.push(dict);
          function_object->tp->nb_call(*this, function_object);
          m_FrameStack.pop_back();

          break;
        }


        const auto function_value = static_cast<FunctionValue*>(function_object.get());
        auto code_object = function_value->code_object;

        std::vector<std::shared_ptr<Variable>> locals;
        locals.reserve(code_object->locals.size());
        for (const auto& local_name : code_object->locals)
        {
          locals.push_back(std::make_shared<Variable>(
            true,
            VALUE_TYPE::UNDEFINED,
            mk_undefined(),
            "__main__",
            local_name,
            false
          ));
        }

        std::size_t positional_index = 0;

        for (const auto& parameter : code_object->params)
        {
          auto& variable = locals[parameter.local_index];
          bool assigned = false;

          if (parameter.kind != ParamKind::KeywordOnly && positional_index < positional_arguments.size())
          {
            const auto value = positional_arguments[positional_index++];
            variable->value = value;
            variable->type = value->type;
            assigned = true;
          }
          else
          {
            const auto it = keyword_arguments.find(parameter.name);
            if (it != keyword_arguments.end())
            {
              const auto value = it->second;
              variable->value = value;
              variable->type = value->type;
              keyword_arguments.erase(it);
              assigned = true;
            }
          }

          if (!assigned)
          {
            if (parameter.has_default)
            {
              const auto value = code_object->constants[parameter.default_const_index];
              variable->value = value;
              variable->type = value->type;
            }
            else
            {
              throw std::runtime_error(std::format("Missing required argument '{}'", parameter.name));
            }
          }
        }

        if (positional_index != positional_arguments.size())
        {
          throw std::runtime_error("Too many positional arguments");
        }

        if (!keyword_arguments.empty())
        {
          const auto unknown = keyword_arguments.begin()->first;
          throw std::runtime_error(std::format("Unexpected keyword argument '{}'", unknown));
        }

        m_FrameStack.back().code_object = code_object;
        m_FrameStack.back().locals = std::move(locals);

        function_object->tp->nb_call(*this, function_object);

        m_FrameStack.pop_back();
        break;
      }
      case CALL:
      {
        const auto positional_arg_count = static_cast<std::size_t>(code.op_codes[idx++]);

        const auto function_object = m_Stack.top();
        m_Stack.pop();

        m_FrameStack.push_back({});

        if (function_object->tp == BuiltinFunctionTypeObject || function_object->tp == TypeObjectTypeObject)
        {
          std::vector<VPtr> positional_arguments;
          positional_arguments.reserve(positional_arg_count);

          for (std::size_t i = 0; i < positional_arg_count; i++)
          {
            positional_arguments.push_back(m_Stack.top());
            m_Stack.pop();
          }

          std::ranges::reverse(positional_arguments);
          m_Stack.push(mk_arr(std::move(positional_arguments)));

          function_object->tp->nb_call(*this, function_object);
          m_FrameStack.pop_back();
          break;
        }

        const auto function_value = static_cast<FunctionValue*>(function_object.get());
        auto code_object = function_value->code_object;

        m_FrameStack.back().code_object = code_object;

        std::vector<std::shared_ptr<Variable>> locals;
        locals.reserve(code_object->locals.size());

        for (const auto& local_name : code_object->locals)
        {
          locals.push_back(std::make_shared<Variable>(
            true,
            VALUE_TYPE::UNDEFINED,
            mk_undefined(),
            "__main__",
            local_name,
            false
          ));
        }

        std::vector<VPtr> positional_arguments;
        positional_arguments.reserve(positional_arg_count);

        for (std::size_t i = 0; i < positional_arg_count; i++)
        {
          positional_arguments.push_back(m_Stack.top());
          m_Stack.pop();
        }

        std::ranges::reverse(positional_arguments);

        std::size_t positional_index = 0;

        for (const auto& parameter : code_object->params)
        {
          if (parameter.kind == ParamKind::KeywordOnly)
          {
            throw std::runtime_error(std::format(
              "Missing required keyword-only argument '{}'",
              parameter.name
            ));
          }

          if (positional_index >= positional_arguments.size())
          {
            throw std::runtime_error(std::format(
              "Missing required argument '{}'",
              parameter.name
            ));
          }

          auto& variable = locals[parameter.local_index];
          const auto value = positional_arguments[positional_index++];

          variable->value = value;
          variable->type = value->type;
        }

        if (positional_index != positional_arguments.size())
        {
          throw std::runtime_error("Too many positional arguments");
        }

        m_FrameStack.back().locals = std::move(locals);

        function_object->tp->nb_call(*this, function_object);
        m_FrameStack.pop_back();
        break;
      }
      case RETURN:
      {
        // m_FrameStack.pop_back();
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

void ByteCodeVM::HandleUnaryOp(UnaryOp compare_type)
{
  const auto value = m_Stack.top();
  m_Stack.pop();

  switch (compare_type)
  {
    case POS:
    {
      m_Stack.push(value->tp->nb_pos(value));
      break;
    }
    case NEG:
    {
      m_Stack.push(value->tp->nb_neg(value));
      break;
    }
    case NOT:
    {
      m_Stack.push(value->tp->nb_not(value));
      break;
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
    throw std::runtime_error("Unhandled binary op");
}

}
