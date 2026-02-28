//
// Created by Максим Литвиненко on 11.09.2025.
//

#include <utility>

#include "yapl/values/CodeObjectValue.hpp"

#include "yapl/Serialization.hpp"


namespace yapl {

CodeObjectValue::CodeObjectValue(std::shared_ptr<CodeObject> code_object)
  :Value(VALUE_TYPE::CODE_OBJECT, CodeObjectTypeObject), code_object(std::move(code_object)) {  }

std::vector<std::byte> CodeObjectValue::Serialize()
{
    std::vector<std::byte> buffer;

    const auto moduleLength = module.size();

    buffer.reserve(2 + 4 + module.size());

    appendByte(buffer, static_cast<std::byte>(VALUE_TYPE::CODE_OBJECT));
    appendUint16(buffer, moduleLength);
    if (moduleLength)
      appendStringBytes(buffer, module);

    auto co_bytes = code_object->Serialize();
    buffer.insert(buffer.end(), co_bytes.begin(), co_bytes.end());

    return buffer;
}

VPtr CodeObjectValue::Deserialize(const std::vector<std::byte> &bytes, std::size_t &offset)
{
  auto module_name_len = readUint16(bytes, offset);
  auto module_name = readStringBytes(bytes, offset, module_name_len);

  auto co = CodeObject::Deserialize(bytes, offset);

  auto b = mk_code_obj(std::make_shared<CodeObject>(co));
  b->module = module_name;

  return b;
}

}
