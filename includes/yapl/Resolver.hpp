//
// Created by lenin on 01.03.2026.
//

#pragma once
#include <unordered_map>

#include "ASTNode.hpp"
#include "Position.hpp"

namespace yapl {
class FunctionArgumentASTNode;

enum class SymbolKind : std::uint8_t
{
  CONSTANT,
  MUTABLE,
  FUNCTION,
  IMPORT,
  PARAMETER,
  TYPE,
};

struct Symbol
{
  std::size_t id;
  std::string name;
  SymbolKind kind;
  bool is_exported;
  bool is_builtin = false;

  std::size_t declaration_scope_id;
  std::size_t declaration_node_id;
  Location declaration_location;

  std::size_t owned_scope_id = -1u; // class User -> scope with methods inside

  std::vector<Location> reference_locations;
  std::vector<std::size_t> reference_node_ids;

  [[nodiscard]] auto IsLocal() const -> bool
  {
    switch (kind)
    {
      case SymbolKind::PARAMETER:
      case SymbolKind::CONSTANT:
      case SymbolKind::MUTABLE: return true;

      default: return false;
    }
  }
};

enum class ScopeKind : std::uint8_t
{
  GLOBAL,
  MODULE,
  FUNCTION,
  BLOCK,
  CLASS
};

struct Scope
{
  std::size_t id;
  ScopeKind kind;

  std::size_t parent_scope_id = -1u; // -1 if no parent
  std::size_t owner_symbol_id = -1u;

  std::size_t node_id;
  std::vector<std::size_t> declared_symbols;
  std::unordered_map<std::string, std::size_t> name_to_symbol;
};

enum class ReferenceKind : std::uint8_t
{
  READ,
  WRITE,
  CALL,
  IMPORT,
  EXPORT,
};

struct SymbolReference
{
  std::size_t node_id;
  std::size_t symbol_id;
  std::size_t scope_id;

  Location location;
  ReferenceKind kind;
};


enum class DiagnosticSeverity : std::uint8_t
{
  INFO,
  WARNING,
  ERROR,
};

enum class DiagnosticCode : std::uint16_t
{
  UNKNOWN_IDENTIFIER,
  DUPLICATE_DECLARATION,
  SHADOWING_DECLARATION,
  CONST_REASSIGNMENT,
  INVALID_RETURN,
  INVALID_BREAK,
  INVALID_CONTINUE,
  USE_BEFORE_DECLARATION,
  USE_BEFORE_INITIALIZATION,
  NOT_CALLABLE,
  ARITY_MISMATCH,
  INTERNAL_RESOLVER_ERROR,
  INVALID_MODULE_SCOPE,
};

struct Diagnostic
{
  DiagnosticSeverity severity;
  DiagnosticCode code;
  std::string message;
  Location location;
};

struct ResolutionResult
{
  std::size_t version = 1;
  std::size_t file_id;

  std::vector<Diagnostic> diagnostics;

  std::vector<Symbol> symbols;
  std::vector<Scope> scopes;

  std::vector<SymbolReference> symbol_references;

  std::unordered_map<std::size_t, std::size_t> node_to_scope;
  std::unordered_map<std::size_t, std::size_t> node_to_symbol;

  // std::unordered_map<std::size_t, std::vector<uint32_t>> symbolToReferenceIndices;

  [[nodiscard]] std::string SerializeJSON() const;
};

class Resolver : public Visitor
{
public:
  explicit Resolver(const std::size_t file_id)
    :m_FileId(file_id) {}

  ResolutionResult Resolve(const RootASTNode& ast_node);

  void visit(const VariableASTNode &node) override;
  void visit(const UnaryOpASTNode &node) override;
  void visit(const BinaryOpASTNode &node) override;
  void visit(const IdentifierASTNode &node) override;


  void visit(const IntegerASTNode &node) override;
  void visit(const FloatASTNode &node) override;
  void visit(const BooleanASTNode &node) override;
  void visit(const StringASTNode &node) override;
  void visit(const ArrayASTNode &node) override;
  void visit(const DictASTNode &node) override;

  void visit(const IfElseExpressionASTNode &node) override;
  void visit(const ScopeASTNode &node) override;
  void visit(const ForLoopASTNode &node) override;
  void visit(const ForEachLoopASTNode &node) override;
  void visit(const WhileLoopASTNode &node) override;
  void visit(const StatementASTNode &node) override;
  void visit(const FunctionASTNode &node) override;
  void visit(const FunctionDeclASTNode &node) override;
  void visit(const ReturnStatementASTNode &node) override;
  void visit(const FunctionCallASTNode& node) override;
  void visit(const FunctionArgumentListASTNode &node) override;
  void visit(const MethodCallASTNode &node) override;
  void visit(const GetPropertyASTNode &node) override;
  void visit(const SetPropertyASTNode &node) override;
  void visit(const ClassASTNode &node) override;
  void visit(const ExportASTNode &node) override;
  void visit(const ImportASTNode &node) override;
  void visit(const IndexASTNode &node) override;
  void visit(const StatementIndexASTNode &node) override;

  void visit(const KeyParamExpressionASTNode &node) override;

private:
  void ReportDuplicateDeclaration(const Token& name_token, const Symbol& prev_symbol);
  void ReportShadowingDeclaration(const Token& name_token, const Symbol& prev_symbol);
  void ReportUnknownIdentifier(const Token& name_token);
  void ReportConstReassignment(const Token& name_token);
  bool EnsureModuleScope(const BaseASTNode& node, std::string_view construct_name);

  void BindNodeToSymbol(std::size_t node_id, std::size_t symbol_id);
  void BindNodeToScope(std::size_t node_id, std::size_t scope_id);

  void ResolveAssignmentTarget(const IdentifierASTNode& node);
  void ResolveCallTarget(const IdentifierASTNode& node);
  void ResolveExportTarget(const IdentifierASTNode& node);
  void DeclareFunctionParameter(const FunctionArgumentASTNode& node);
  void DeclareImportedSymbol(const Token& name_token, std::size_t declaration_node_id);
  std::size_t DeclareBuiltinSymbol(std::string_view name, SymbolKind kind);

  std::size_t LookupInParentScopes(std::string_view name) const;
  std::size_t LookupInCurrentOrParentScopes(std::string_view name) const;

  std::size_t PushScope(ScopeKind kind, std::size_t node_id, std::size_t parent_scope_id);
  void PopScope();

  Scope& CurrentScope();
  const Scope& CurrentScope() const;
  std::size_t CurrentScopeId() const;

  void SeedBuiltins();

  Location ToLocation(const BaseASTNode& ast) const;
  Location ToLocation(const Token& node) const;

private:
  std::vector<std::size_t> m_ScopeStack;

  ResolutionResult m_Result = {};

  std::size_t m_ScopeId = 0;
  std::size_t m_SymbolId = 0;
  std::size_t m_FileId;
};

std::string to_string(DiagnosticSeverity value);
std::string to_string(DiagnosticCode value);
std::string to_string(SymbolKind value);
std::string to_string(ScopeKind value);
std::string to_string(ReferenceKind value);


}
