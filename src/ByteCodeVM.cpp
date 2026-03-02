//
// Created by Максим Литвиненко on 26.08.2025.
//

#include "yapl/ByteCodeVM.hpp"

#include <algorithm>
#include <fstream>
#include <yapl/Lexer.hpp>
#include <yapl/Parser.hpp>
#include <yapl/values/ArrayValue.hpp>
#include <yapl/values/UserDefinedValue.hpp>

#include "yapl/Utils.hpp"
#include "yapl/values/CodeObjectValue.hpp"
#include "yapl/values/UndefinedValue.hpp"
#include "yapl/values/FunctionValue.hpp"
#include "yapl/values/ModuleValue.hpp"


#ifdef __linux__
#define DEBUG std::raise(SIGINT);
#endif
#ifdef __APPLE__
#include <csignal>
#define DEBUG std::raise(SIGINT);
#endif
#ifdef __MINGW32__
#define DEBUG __debugbreak();
#endif
#ifdef _MSC_VER
#define DEBUG __debugbreak();
#endif

namespace yapl {

static std::shared_ptr<Variable> FindInFrame(Frame& frame, const std::string& name)
{
  if (frame.names.contains(name)) return frame.names.at(name);
  for (const auto& local : frame.locals)
  {
    if (local && local->name == name) return local;
  }
  return nullptr;
}

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
        m_Stack.push(mk_undefined());
        break;
      }
      case MAKE_ARR:
      {
        int amount = code.op_codes[idx++];
        std::vector<std::shared_ptr<Value>> values;
        values.reserve(amount);
        for (int i = 0; i < amount; i++)
        {
          values.push_back(m_Stack.top());
          m_Stack.pop();
        }
        m_Stack.push(mk_arr(std::move(values)));

        break;
      }
      case MAKE_DICT:
      {
        std::size_t amount = code.op_codes[idx++];

        auto dict = mk_dict();
        for (std::size_t i = 0; i < amount; i++)
        {
          auto value = m_Stack.top();
          m_Stack.pop();
          auto key = m_Stack.top();
          m_Stack.pop();
          dict->value[key] = value;
        }
        m_Stack.push(dict);

        break;
      }
      case GET_ITER:
      {
        auto value = m_Stack.top();
        m_Stack.pop();

        auto iter = value->tp->nb_iter(value);
        m_Stack.push(iter);

        break;
      }
      case FOR_ITER:
      {
        int jmp_amount = static_cast<int>(code.op_codes[idx++]);
        try
        {
          auto value = m_Stack.top();
          auto next = value->tp->nb_next(value);
          m_Stack.push(next);
        }
        catch (StopIteration&)
        {
          m_Stack.pop();
          idx += jmp_amount;
        }
        break;
      }
      case INIT_VAR:
      {
        const auto& frame = *m_FrameStack.back();

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
        auto& frame = *m_FrameStack.back();

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
        auto& frame = *m_FrameStack.back();

        const auto name_idx = code.op_codes[idx++];
        const auto& name = code.names[name_idx];

        if (name == "debug")
        {
          DEBUG
          break;
        }

        auto var = FindInFrame(frame, name);
        if (!var && frame.globals) var = FindInFrame(*frame.globals, name);
        if (!var && m_Globals.contains(name)) var = m_Globals.at(name);

        m_Stack.push(var ? var->value : mk_undefined());
        break;
      }
      case LOAD_LOCAL:
      {
        auto& frame = *m_FrameStack.back();

        const auto locals_index = code.op_codes[idx++];
        const auto var = frame.locals[locals_index];
        m_Stack.push(var->value);
        break;
      }
      case STORE_LOCAL:
      {
        auto& frame = *m_FrameStack.back();

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
        auto& frame = *m_FrameStack.back();

        const auto names_index = code.op_codes[idx++];
        const auto& name = code.names[names_index];

        auto var = FindInFrame(frame, name);
        if (!var && frame.globals) var = FindInFrame(*frame.globals, name);
        if (!var && m_Globals.contains(name)) var = m_Globals.at(name);

        const auto value = m_Stack.top();
        m_Stack.pop();

        if (!var)
          throw std::runtime_error(std::format("Unknown variable {}", name));

        var->value = value;
        var->type = value->type;
        break;
      }
      case MAKE_FUNC:
      {
        auto& frame = *m_FrameStack.back();

        const auto code_object_value = m_Stack.top();
        const auto casted = static_cast<CodeObjectValue*>(code_object_value.get());
        const auto name = casted->code_object->name;
        m_Stack.pop();

        auto fn = mk_func(name, static_cast<CodeObjectValue*>(code_object_value.get())->code_object, frame.globals ? frame.globals : m_FrameStack.back());

        frame.names[name] = std::make_shared<Variable>(true, VALUE_TYPE::FUNCTION, fn, frame.code_object ? frame.code_object->name : "__main__", name, false);
        break;
      }
      case KW_CALL:
      {
        const int positional_arg_count = code.op_codes[idx++];
        const int keyword_arg_count    = code.op_codes[idx++];

        const auto function_object = m_Stack.top();
        m_Stack.pop();

        std::unordered_map<std::string, VPtr> keyword_arguments;
        keyword_arguments.reserve(keyword_arg_count);
        for (int i = 0; i < keyword_arg_count; i++)
        {
          const auto kname = static_cast<StringValue*>(m_Stack.top().get())->value; m_Stack.pop();
          const auto kval  = m_Stack.top(); m_Stack.pop();
          keyword_arguments.emplace(kname, kval);
        }

        std::vector<VPtr> positional_arguments;
        positional_arguments.reserve(positional_arg_count);
        for (int i = 0; i < positional_arg_count; i++)
          { positional_arguments.push_back(m_Stack.top()); m_Stack.pop(); }
        std::ranges::reverse(positional_arguments);

        InvokeFunction(function_object,
                       std::move(positional_arguments),
                       std::move(keyword_arguments));
        break;
      }
      case CALL:
      {
        const auto positional_arg_count = static_cast<std::size_t>(code.op_codes[idx++]);
        const auto function_object = m_Stack.top();
        m_Stack.pop();

        std::vector<VPtr> positional_arguments;
        positional_arguments.reserve(positional_arg_count);
        for (std::size_t i = 0; i < positional_arg_count; i++)
          { positional_arguments.push_back(m_Stack.top()); m_Stack.pop(); }
        std::ranges::reverse(positional_arguments);

        InvokeFunction(function_object, std::move(positional_arguments));
        break;
      }
      case KW_CALL_METHOD:
      {
        const int  names_idx            = code.op_codes[idx++];
        const auto method_name     = code.names[names_idx];
        const int  positional_arg_count = code.op_codes[idx++];
        const int  keyword_arg_count    = code.op_codes[idx++];

        const auto self = m_Stack.top();
        m_Stack.pop();

        const auto method_it = self->tp->methods.find(method_name);
        if (method_it == self->tp->methods.end())
          throw std::runtime_error(std::format(
            "Type '{}' has no method '{}'", self->tp->name, method_name));

        const auto function_object = method_it->second;

        std::unordered_map<std::string, VPtr> keyword_arguments;
        keyword_arguments.reserve(keyword_arg_count);
        for (int i = 0; i < keyword_arg_count; i++)
        {
          const auto kname = static_cast<StringValue*>(m_Stack.top().get())->value; m_Stack.pop();
          const auto kval  = m_Stack.top(); m_Stack.pop();
          keyword_arguments.emplace(kname, kval);
        }

        std::vector<VPtr> positional_arguments;
        positional_arguments.reserve(positional_arg_count + 1);
        for (int i = 0; i < positional_arg_count; i++)
          { positional_arguments.push_back(m_Stack.top()); m_Stack.pop(); }
        positional_arguments.push_back(self);
        std::ranges::reverse(positional_arguments);

        InvokeFunction(function_object,
                       std::move(positional_arguments),
                       std::move(keyword_arguments));
        break;
      }

      case CALL_METHOD:
      {
        const int  names_idx            = code.op_codes[idx++];
        const auto method_name          = code.names[names_idx];
        const auto positional_arg_count = static_cast<std::size_t>(code.op_codes[idx++]);

        const auto self = m_Stack.top();
        m_Stack.pop();

        const auto method_it = self->tp->methods.find(method_name);
        if (method_it == self->tp->methods.end())
          throw std::runtime_error(std::format(
            "Type '{}' has no method '{}'", self->tp->name, method_name));

        const auto function_object = method_it->second;

        std::vector<VPtr> positional_arguments;
        positional_arguments.reserve(positional_arg_count + 1);
        for (std::size_t i = 0; i < positional_arg_count; i++)
          { positional_arguments.push_back(m_Stack.top()); m_Stack.pop(); }
        positional_arguments.push_back(self);
        std::ranges::reverse(positional_arguments);

        InvokeFunction(function_object, std::move(positional_arguments));
        break;
      }
      case GET_PROPERTY:
      {
        const auto name_idx = code.op_codes[idx++];
        const auto& attr_name = code.names[name_idx];

        const auto obj = m_Stack.top();
        m_Stack.pop();

        if (!obj->tp->nb_getattr)
          throw std::runtime_error(std::format(
            "Type '{}' does not support attribute access", obj->tp->name));

        const auto result = obj->tp->nb_getattr(obj, attr_name);
        m_Stack.push(result);
        break;
      }
      case SET_PROPERTY:
      {
        const auto name_idx = code.op_codes[idx++];
        const auto& attr_name = code.names[name_idx];

        const auto obj = m_Stack.top();
        m_Stack.pop();
        const auto value = m_Stack.top();
        m_Stack.pop();

        if (!obj->tp->nb_setattr)
          throw std::runtime_error(std::format(
            "Type '{}' does not support attribute assignment", obj->tp->name));

        obj->tp->nb_setattr(obj, attr_name, value);
        break;
      }
      case MAKE_TYPE:
      {
        const auto name_idx = code.op_codes[idx++];
        const auto method_amount = code.op_codes[idx++];
        const auto name = code.names[name_idx];

        auto& frame = *m_FrameStack.back();
        auto globals = frame.globals ? frame.globals : m_FrameStack.back();

        std::unordered_map<std::string, VPtr> methods;
        for (std::size_t i = 0; i < method_amount; ++i)
        {
          const auto method_name = m_Stack.top();
          m_Stack.pop();
          auto casted = static_cast<StringValue*>(method_name.get());
          const auto code_object_value = m_Stack.top();
          m_Stack.pop();

          auto fn = mk_func(casted->value, static_cast<CodeObjectValue*>(code_object_value.get())->code_object, globals);
          methods[casted->value] = fn;
        }

        TypeObject* user_type = new TypeObject {
          .name = name,
          .methods = std::move(methods),
          .nb_make = [&user_type, this](const std::vector<VPtr>& args) -> VPtr
          {
            const auto new_value  = std::make_shared<UserDefinedValue>(user_type);
            const auto& constructor = user_type->methods["init"];

            std::vector<VPtr> init_args;
            init_args.reserve(args.size() + 1);
            init_args.push_back(new_value);
            for (const auto& v : args) init_args.push_back(v);

            InvokeFunction(constructor, std::move(init_args));
            return new_value;
          }
        };

        init_base_methods(user_type);
        m_Types.push_back(user_type);
        const auto user_type_value = mk_type(user_type);
        frame.names[name] = std::make_shared<Variable>(true, VALUE_TYPE::TYPE, user_type_value, frame.code_object ? frame.code_object->name : "__main__", name);

        break;
      }
      case LOAD_MODULE:
      {
        auto _module_name = m_Stack.top();
        auto module_name = static_cast<StringValue*>(_module_name.get())->value;
        m_Stack.pop();

        LoadModule(module_name);

        break;
      }
      case IMPORT_NAME:
      {
        auto& frame = *m_FrameStack.back();

        auto _frame = m_Stack.top();
        auto module_frame = static_cast<ModuleValue*>(_frame.get())->module;

        const int names_idx = code.op_codes[idx++];
        auto name = code.names[names_idx];

        auto found = FindInFrame(*module_frame, name);
        frame.names[name] = std::make_shared<Variable>(
          true,
          found ? found->type : VALUE_TYPE::UNDEFINED,
          found ? found->value : mk_undefined(),
          module_frame->code_object ? module_frame->code_object->name : "__module__",
          name
        );
        break;
      }
      case GET_INDEX:
      {
        auto index = m_Stack.top();
        m_Stack.pop();
        auto expr = m_Stack.top();
        m_Stack.pop();

        auto result = expr->OperatorIndex(index);
        m_Stack.push(result);
        break;
      }
      case SET_INDEX:
      {
        auto RHS = m_Stack.top();
        m_Stack.pop();
        auto index = m_Stack.top();
        m_Stack.pop();
        auto expr = m_Stack.top();
        m_Stack.pop();

        expr->OperatorIndexSet(index, RHS);
        break;
      }
      case POP:
      {
        m_Stack.pop();
        break;
      }

      case RETURN:
      {
        return;
      }
      case HALT:
      {
        return;
      }

      default:
        throw std::runtime_error("Unhandled Op");
    }
  }
}

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
    case ADD: op  = lhs->tp->nb_add; rop = rhs->tp->nb_add; break;
    case MUL: op  = lhs->tp->nb_mul; rop = rhs->tp->nb_mul; break;
    case SUB: op  = lhs->tp->nb_sub; rop = rhs->tp->nb_sub; break;
    case DIV: op  = lhs->tp->nb_div; rop = rhs->tp->nb_div; break;
    case MOD: op  = lhs->tp->nb_mod; rop = rhs->tp->nb_mod; break;
    case LT:  op  = lhs->tp->nb_lt;  rop = rhs->tp->nb_lt;  break;
    case GT:  op  = lhs->tp->nb_gt;  rop = rhs->tp->nb_gt;  break;
    case LQ:  op  = lhs->tp->nb_le;  rop = rhs->tp->nb_le;  break;
    case GQ:  op  = lhs->tp->nb_ge;  rop = rhs->tp->nb_ge;  break;
    case EQ:  op  = lhs->tp->nb_eq;  rop = rhs->tp->nb_eq;  break;
    case NEQ: op  = lhs->tp->nb_nq;  rop = rhs->tp->nb_nq;  break;
    case AND: op  = lhs->tp->nb_and; rop = rhs->tp->nb_and; break;
    case OR:  op  = lhs->tp->nb_or;  rop = rhs->tp->nb_or;  break;
    default: break;
  }

  if (op) {
    VPtr r = op(lhs, rhs);
    if (r != NotImplemented) { m_Stack.push(r); return; }
  }
  if (rop) {
    VPtr r = rop(rhs, lhs);
    if (r != NotImplemented) { m_Stack.push(r); return; }
  }
  throw std::runtime_error(std::format("Unhandled binary op {} for types {} and {}", binary_op_to_string(compare_type), lhs->tp->name, rhs->tp->name));
}

void ByteCodeVM::InvokeFunction(
    const VPtr& function_object,
    std::vector<VPtr> positional_arguments,
    std::unordered_map<std::string, VPtr> keyword_arguments)
{
  m_FrameStack.push_back(std::make_shared<Frame>());
  auto& call_frame = *m_FrameStack.back();

  if (function_object->tp == BuiltinFunctionTypeObject ||
      (function_object->tp->nb_call && function_object->tp != FunctionTypeObject))
  {
    auto dict = mk_dict();
    for (const auto& [key, value] : keyword_arguments)
      dict->OperatorIndexSet(mk_str(key), value);

    m_Stack.push(mk_arr(std::move(positional_arguments)));
    m_Stack.push(dict);
    function_object->tp->nb_call(*this, function_object);
    m_FrameStack.pop_back();
    return;
  }

  const auto fn = static_cast<FunctionValue*>(function_object.get());
  auto code_object = fn->code_object;

  call_frame.globals = fn->globals;

  std::vector<std::shared_ptr<Variable>> locals;
  locals.reserve(code_object->locals.size());
  for (const auto& local_name : code_object->locals)
    locals.push_back(std::make_shared<Variable>(
        true, VALUE_TYPE::UNDEFINED, mk_undefined(),
        call_frame.globals && call_frame.globals->code_object ? call_frame.globals->code_object->name : "__main__",
        local_name, false));

  std::size_t positional_index = 0;
  for (const auto& parameter : code_object->params)
  {
    auto& variable = locals[parameter.local_index];
    bool assigned = false;

    if (parameter.kind != ParamKind::KeywordOnly &&
        positional_index < positional_arguments.size())
    {
      const auto value = positional_arguments[positional_index++];
      variable->value  = value;
      variable->type   = value->type;
      assigned = true;
    }
    else
    {
      const auto it_kw = keyword_arguments.find(parameter.name);
      if (it_kw != keyword_arguments.end())
      {
        variable->value = it_kw->second;
        variable->type  = it_kw->second->type;
        keyword_arguments.erase(it_kw);
        assigned = true;
      }
    }

    if (!assigned)
    {
      if (parameter.has_default)
      {
        const auto value = code_object->constants[parameter.default_const_index];
        variable->value  = value;
        variable->type   = value->type;
      }
      else
        throw std::runtime_error(
            std::format("Missing required argument '{}'", parameter.name));
    }
  }

  if (positional_index != positional_arguments.size())
    throw std::runtime_error("Too many positional arguments");

  if (!keyword_arguments.empty())
    throw std::runtime_error(
        std::format("Unexpected keyword argument '{}'",
                    keyword_arguments.begin()->first));

  call_frame.code_object = code_object;
  call_frame.locals = std::move(locals);

  function_object->tp->nb_call(*this, function_object);
  m_FrameStack.pop_back();
}

void ByteCodeVM::LoadModule(const std::string& module_name)
{
  const auto filename = module_name + ".yapl";
  const auto full_path = base_path / filename;

  CodeObject co;
  // if (!is_valid_cache(full_path))
  if (true)
  {
    std::ifstream t(full_path);
    std::string text((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    Lexer lexer {text};
    auto tokens = lexer.make_tokens();

    auto lines = get_lines_from_text(text);
    Parser parser {tokens, filename, lines};
    auto ast = parser.parse_root();
    auto ast_as_root = static_cast<RootASTNode*>(ast.get());

    ByteCodeVisitor visitor;

    co = visitor.visit_RootASTNode(*ast_as_root);

    auto co_bytes = co.Serialize();
    auto bytes = serialize_module(co_bytes, full_path);
    auto cache_path = base_path / ".cache";
    std::filesystem::create_directories(cache_path);

    std::ofstream cache(cache_path / (module_name + ".yaplcache"), std::ios::binary);
    cache.write(
      reinterpret_cast<const char*>(bytes.data()),
      static_cast<std::streamsize>(bytes.size())
    );
  }
  else
  {
    auto cache_path = base_path / ".cache" / (module_name + ".yaplcache");
    std::ifstream fin(cache_path, std::ios::binary);
    fin.seekg(33, std::ios::beg);

    fin.seekg(0, std::ios::end);
    const std::streamsize size = fin.tellg();
    fin.seekg(33, std::ios::beg);

    std::vector<std::byte> bytes(size - 33);
    fin.read(reinterpret_cast<char*>(bytes.data()), size);

    std::size_t offset = 0;
    co = CodeObject::Deserialize(bytes, offset);
  }

  std::vector<std::shared_ptr<Variable>> locals;
  locals.reserve(co.locals.size());
  for (const auto& name : co.locals)
  {
    locals.push_back(std::make_shared<Variable>(true, VALUE_TYPE::UNDEFINED, nullptr, module_name, name));
  }

  auto module_frame = std::make_shared<Frame>();
  module_frame->code_object = std::make_shared<CodeObject>(co);
  module_frame->locals = std::move(locals);
  module_frame->globals = module_frame;

  m_FrameStack.push_back(module_frame);
  Run(co);
  m_FrameStack.pop_back();

  modules[module_name] = module_frame;

  auto frame_value = mk_module(module_frame);
  m_Stack.push(frame_value);
}

}
