//
// Created by lenin on 07.03.2026.
//

#pragma once
#include <filesystem>
#include <memory>
#include <span>
#include <string>

#include "ASTNode.hpp"
#include "ByteCodeVM.hpp"
#include "CodeObject.hpp"
#include "Resolver.hpp"

namespace yapl {
struct Frame;

class Module
{
public:
  std::filesystem::path path;
  std::string name;

public:
  explicit Module(const std::filesystem::path& path, const std::string& name);
  explicit Module(const std::filesystem::path& path, const std::string& name, const std::string& text);

  auto IsCacheValid() const -> bool;
  auto SerializeCache() const -> void;
  auto DeserializeCache() -> void;

  auto Tokenize() -> void;
  auto Parse() -> void;
  auto Resolve() -> std::shared_ptr<ResolutionResult>;
  auto Generate() -> void;
  auto Run(ByteCodeVM& VM, const std::shared_ptr<Frame> &globals = nullptr) const -> std::shared_ptr<Frame>;

  auto GetCodeObject() -> std::shared_ptr<CodeObject>
  {
    return m_CodeObject;
  }
  auto GetLines() -> std::span<std::string>
  {
    return m_SourceLines;
  }

private:
  std::string m_Text;
  std::vector<Token> m_Tokens;
  std::unique_ptr<BaseASTNode> m_AST;
  std::shared_ptr<ResolutionResult> m_ResolutionResult;
  std::shared_ptr<CodeObject> m_CodeObject;
  std::vector<std::string> m_SourceLines;

  std::filesystem::path m_Path;
  std::string m_Filename;
};

}
