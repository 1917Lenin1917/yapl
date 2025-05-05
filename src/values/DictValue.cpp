//
// Created by lenin on 04.05.2025.
//

#include "yapl/values/DictValue.hpp"

namespace yapl {


DictValue::DictValue()
  :Value(VALUE_TYPE::DICT, DictTypeObject)
{

}

std::string DictValue::print() const
{
  if (value.empty())
    return "{}";

  std::string res = "{";
  for (const auto& [k, v] : value)
  {
    res += std::format("{}: {}, ", k->print(), v->print());
  }
  res.erase(res.end()-2, res.end());
  res += "}";
  return res;
}

std::unique_ptr<Value> DictValue::Copy() const
{
  // FIXME:
  return nullptr;
}


std::shared_ptr<Value> DictValue::OperatorIndex(const std::shared_ptr<Value> &idx)
{
  return value[idx];
}

void DictValue::OperatorIndexSet(const std::shared_ptr<Value> &idx, std::shared_ptr<Value> new_val)
{
  value[idx] = new_val;
}






void init_dict_methods(TypeObject *tp)
{

}


}
