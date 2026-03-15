//
// Created by lenin on 07.03.2026.
//

#include "yapl/Module.hpp"

#include "yapl/ByteCodeVisitor.hpp"
#include "yapl/Lexer.hpp"
#include "yapl/Parser.hpp"
#include "yapl/Resolver.hpp"
#include "yapl/Utils.hpp"
#include "yapl/values/ModuleValue.hpp"

namespace yapl {
Module::Module(const std::filesystem::path &path, const std::string &name)
  :m_Path(path), m_Filename(name)
{
  std::ifstream source_file(m_Path / (m_Filename + ".yapl"));
  m_Text = {
    (std::istreambuf_iterator<char>(source_file)),
    std::istreambuf_iterator<char>()
  };
  m_SourceLines = get_lines_from_text(m_Text);
}

Module::Module(const std::filesystem::path &path, const std::string &name, const std::string &text)
  :m_Text(text), m_Path(path), m_Filename(name)
{
  m_SourceLines = get_lines_from_text(m_Text);
}

auto Module::IsCacheValid() const -> bool
{
  // return is_valid_cache(m_Path / (m_Filename + ".yapl"));
  return false;
}

auto Module::SerializeCache() const -> void
{
  std::vector<std::byte> bytes = m_CodeObject->Serialize();

  const auto cache_path = m_Path / ".cache";

  std::filesystem::create_directories(cache_path);

  auto cache_file_path = cache_path / m_Filename;
  cache_file_path.replace_extension("yaplcache");

  std::ofstream cache_file(cache_file_path, std::ios::binary);
  cache_file.write(
    reinterpret_cast<const char *>(bytes.data()),
    static_cast<std::streamsize>(bytes.size())
  );
}

auto Module::DeserializeCache() -> void
{
  auto cache_path = m_Path / ".cache" / m_Filename;
  cache_path.replace_extension("yaplcache");

  std::ifstream cache_file(cache_path, std::ios::binary);
  cache_file.seekg(33, std::ios::beg);

  cache_file.seekg(0, std::ios::end);
  const std::streamsize size = cache_file.tellg();
  cache_file.seekg(33, std::ios::beg);

  std::vector<std::byte> bytes(size - 33);
  cache_file.read(reinterpret_cast<char *>(bytes.data()), size);

  std::size_t offset = 0;
  m_CodeObject = std::make_shared<CodeObject>(CodeObject::Deserialize(bytes, offset));
}

auto Module::Tokenize() -> void
{
  Lexer lexer{ m_Text };
  m_Tokens = lexer.Tokenize();
}

auto Module::Parse() -> void
{
  Parser parser{ m_Tokens, m_Filename, m_SourceLines };
  m_AST = parser.Parse();
}

auto Module::Resolve() -> std::shared_ptr<ResolutionResult>
{
  Resolver resolver{ 0 };
  m_ResolutionResult = std::make_shared<ResolutionResult>(
    resolver.Resolve(*static_cast<RootASTNode*>(m_AST.get()))
  );

  return m_ResolutionResult;
}

auto Module::Generate() -> void
{
  ByteCodeVisitor visitor{ *m_ResolutionResult };
  m_CodeObject = visitor.visit_RootASTNode(*static_cast<RootASTNode*>(m_AST.get()));
	m_CodeObject->name = m_Filename;
}

auto Module::Run(ByteCodeVM &VM, const std::shared_ptr<Frame> &globals) const -> std::shared_ptr<Frame>
{
  std::vector<std::shared_ptr<Variable>> locals;
  locals.reserve(m_CodeObject->locals.size());
  for (const auto& var_name : m_CodeObject->locals)
  {
    locals.push_back(std::make_shared<Variable>(Variable{
      // TODO: not all locals are const and also this is never used
      // TODO: so we should probably keep local -> symbol table somewhere
      .is_const = true,
      .type = VALUE_TYPE::UNDEFINED,
      .value = nullptr,
      .module_name = name,
      .name = var_name,
      .is_tdz = true,
    }));
  }

  auto module_frame = std::make_shared<Frame>(Frame{
    .code_object = m_CodeObject,
    .locals = std::move(locals),
  });
  module_frame->globals = globals ? globals : nullptr;

  VM.m_FrameStack.push_back(module_frame);
  VM.Run(*m_CodeObject);
  VM.m_FrameStack.pop_back();

  return module_frame;
}

}
