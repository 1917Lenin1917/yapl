//
// Created by lenin on 18.05.2025.
//

#include <utility>
#include <yapl/values/FunctionValue.hpp>
#include <yapl/values/ArrayValue.hpp>

namespace yapl {

FunctionValue::FunctionValue(std::string name, FunctionASTNode *fn)
    :Value(VALUE_TYPE::FUNCTION, FunctionTypeObject), name(std::move(name)), fn(fn) {}

std::unique_ptr<Value> FunctionValue::Copy() const
{
    return nullptr;
}



void init_function_methods(TypeObject* tp)
{

  tp->nb_call = [](Visitor& visitor, const VPtr& self, const std::vector<std::unique_ptr<BaseASTNode>>& args)->VPtr
  {
    // return self_fn->fn->body->visit(visitor);
    auto self_fn = as_func(self.get());
    auto func_def = self_fn->fn;
    auto arg_list = dynamic_cast<FunctionArgumentListASTNode*>(dynamic_cast<FunctionDeclASTNode*>(func_def->decl.get())->args.get());
    // check if we have variadic args
    // TODO: handle kwargs
    if (const auto& args_arg = arg_list->args_arg)
    {
        auto list = std::vector<VPtr>{  };
        for (const auto& arg : args)
        {
            // TODO: this dynamic_cast is probably really slow!
            if (const auto& str_expr = dynamic_cast<StarredExpressionASTNode*>(arg.get()))
            {
                auto v = str_expr->visit(visitor);
                for (const auto& val : as_arr(v.get())->value)
                {
                    list.push_back(val);
                }
                continue;
            }
            list.push_back(arg->visit(visitor));
        }
        auto VPtrList = std::make_unique<ArrayValue>(list);
        auto func = visitor.interpreter.PushFunction(std::format("{}::operator()", self->print()));
        func->set_argument(std::make_unique<Variable>(false, VPtrList->type, std::move(VPtrList)), arg_list->args_arg->name.value);

        func_def->body->visit(visitor);
        auto ret_value = func->return_value;
        visitor.interpreter.PopScope();
        visitor.interpreter.pop_function();

        return ret_value;
    }

    // TODO: add starred expression handling
    size_t arg_amount = arg_list->get_argument_amount();
    if (arg_amount != -1 && args.size() != arg_amount)
    {
        std::cerr << std::format("Invalid amount of arguments provided to function {}. Expected {} arguments, instead got {}.\n", self->print(), arg_amount, args.size());
        return nullptr;
    }
    std::vector<VPtr> values;
    // set args with passed expressions
    for (const auto & arg : args)
    {
        auto value = arg->visit(visitor);
        values.push_back(value);
    }


    auto prev_module = visitor.interpreter.current_module;
    if (!self->module.empty())
    {
        visitor.interpreter.current_module = visitor.interpreter.GetModule(self->module);
    }
    auto func = visitor.interpreter.PushFunction(self->print());
    for (int i = 0; i < values.size(); i++)
    {
        auto value = values[i];
        func->set_argument(std::make_unique<Variable>(false, value->type, value), arg_list->get_argument_name(i));
    }
    func_def->body->visit(visitor);
    auto ret_value = func->return_value;
    visitor.interpreter.PopScope();
    visitor.interpreter.pop_function();
    if (!self->module.empty())
    {
        visitor.interpreter.current_module = prev_module;
    }

    return ret_value;

  };
}

}