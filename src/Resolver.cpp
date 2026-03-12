#include "yapl/Resolver.hpp"

#include "yapl/values/ArrayValue.hpp"
#include "yapl/values/DictValue.hpp"
#include "yapl/values/FunctionValue.hpp"
#include "yapl/values/IntegerValue.hpp"
#include "yapl/values/TypeObjectValue.hpp"

namespace yapl {
std::string ResolutionResult::SerializeJSON() const
{
  return "TBD";
}

ResolutionResult Resolver::Resolve(const RootASTNode& ast_node)
{
  m_Result = {};
  m_ScopeStack.clear();
  m_ScopeId = 0;
  m_SymbolId = 0;

  const std::size_t module_scope_id = PushScope(
    ScopeKind::MODULE,
    ast_node.id,
    -1ull
  );

  BindNodeToScope(ast_node.id, module_scope_id);

  SeedBuiltins();

  for (const auto& child_node : ast_node.nodes) {
    child_node->visit(*this);
  }

  PopScope();

  return std::move(m_Result);
}

void Resolver::visit(const VariableASTNode& node)
{
  BindNodeToScope(node.id, CurrentScopeId());

  if (node.value != nullptr) {
    node.value->visit(*this);
  }

  auto& scope = CurrentScope();

  const auto iterator = scope.name_to_symbol.find(node.name.value);
  if (iterator != scope.name_to_symbol.end()) {
    const auto& previous_symbol = m_Result.symbols[iterator->second];
    ReportDuplicateDeclaration(node.name, previous_symbol);
    return;
  }

  if (const auto shadowed_symbol_id = LookupInParentScopes(node.name.value); shadowed_symbol_id != -1ull) {
    ReportShadowingDeclaration(node.name, m_Result.symbols[shadowed_symbol_id]);
  }

  const std::size_t symbol_id = m_SymbolId++;

  const auto symbol_kind =
    node.type.type == TOKEN_TYPE::CONST
      ? SymbolKind::CONSTANT
      : SymbolKind::MUTABLE;

  m_Result.symbols.push_back(Symbol{
    .id = symbol_id,
    .name = node.name.value,
    .kind = symbol_kind,
    .is_exported = false,
    .declaration_scope_id = scope.id,
    .declaration_node_id = node.id,
    .declaration_location = ToLocation(node.name),
    .reference_locations = {},
    .reference_node_ids = {},
  });

  scope.name_to_symbol.emplace(node.name.value, symbol_id);
  scope.declared_symbols.push_back(symbol_id);

  BindNodeToSymbol(node.id, symbol_id);
}

void Resolver::visit(const UnaryOpASTNode &node)
{
  BindNodeToScope(node.id, CurrentScopeId());
  node.RHS->visit(*this);
}

void Resolver::visit(const BinaryOpASTNode &node)
{
  BindNodeToScope(node.id, CurrentScopeId());
  node.LHS->visit(*this);
  node.RHS->visit(*this);
}

void Resolver::visit(const IdentifierASTNode& node)
{
  BindNodeToScope(node.id, CurrentScopeId());

  const std::size_t symbol_id = LookupInCurrentOrParentScopes(node.token.value);
  if (symbol_id == -1ull) {
    ReportUnknownIdentifier(node.token);
    return;
  }

  auto& symbol = m_Result.symbols[symbol_id];

  BindNodeToSymbol(node.id, symbol_id);

  symbol.reference_node_ids.push_back(node.id);
  symbol.reference_locations.push_back(ToLocation(node));

  m_Result.symbol_references.push_back(SymbolReference{
    .node_id = node.id,
    .symbol_id = symbol_id,
    .scope_id = CurrentScopeId(),
    .location = ToLocation(node),
    .kind = ReferenceKind::READ,
  });
}

void Resolver::visit(const IntegerASTNode &node)
{
  BindNodeToScope(node.id, CurrentScopeId());
}

void Resolver::visit(const FloatASTNode &node)
{
  BindNodeToScope(node.id, CurrentScopeId());
}

void Resolver::visit(const BooleanASTNode &node)
{
  BindNodeToScope(node.id, CurrentScopeId());
}

void Resolver::visit(const StringASTNode &node)
{
  BindNodeToScope(node.id, CurrentScopeId());
}

void Resolver::visit(const ArrayASTNode &node)
{
  BindNodeToScope(node.id, CurrentScopeId());

  for (const auto& value : node.values)
    value->visit(*this);
}

void Resolver::visit(const DictASTNode &node)
{
}

void Resolver::visit(const IfElseExpressionASTNode& node)
{
  BindNodeToScope(node.id, CurrentScopeId());

  node.condition->visit(*this);

  const std::size_t parent_scope_id = CurrentScopeId();

  const std::size_t true_scope_id = PushScope(
    ScopeKind::BLOCK,
    node.true_scope->id,
    parent_scope_id
  );
  BindNodeToScope(node.true_scope->id, true_scope_id);
  node.true_scope->visit(*this);
  PopScope();

  if (node.false_scope != nullptr)
  {
    const std::size_t false_scope_id = PushScope(
      ScopeKind::BLOCK,
      node.false_scope->id,
      parent_scope_id
    );
    BindNodeToScope(node.false_scope->id, false_scope_id);
    node.false_scope->visit(*this);
    PopScope();
  }
}

void Resolver::visit(const ScopeASTNode &node)
{
  BindNodeToScope(node.id, CurrentScopeId());
  for (const auto& child_node : node.nodes)
  {
    child_node->visit(*this);
  }
}

void Resolver::visit(const ForLoopASTNode& node)
{
  BindNodeToScope(node.id, CurrentScopeId());

  const std::size_t parent_scope_id = CurrentScopeId();

  const std::size_t loop_scope_id = PushScope(
    ScopeKind::BLOCK,
    node.id,
    parent_scope_id
  );

  node.declaration->visit(*this);
  node.condition->visit(*this);
  node.increment->visit(*this);

  BindNodeToScope(node.scope->id, loop_scope_id);
  node.scope->visit(*this);

  PopScope();
}

void Resolver::visit(const ForEachLoopASTNode &node) { }

void Resolver::visit(const WhileLoopASTNode& node)
{
  BindNodeToScope(node.id, CurrentScopeId());

  node.condition->visit(*this);

  const std::size_t parent_scope_id = CurrentScopeId();

  const std::size_t loop_scope_id = PushScope(
    ScopeKind::BLOCK,
    node.id,
    parent_scope_id
  );

  BindNodeToScope(node.scope->id, loop_scope_id);
  node.scope->visit(*this);

  PopScope();
}

void Resolver::visit(const StatementASTNode& node)
{
  BindNodeToScope(node.id, CurrentScopeId());

  const auto* identifier_node = static_cast<IdentifierASTNode*>(node.base.get());
  ResolveAssignmentTarget(*identifier_node);

  if (node.RHS != nullptr) {
    node.RHS->visit(*this);
  }
}

void Resolver::visit(const FunctionASTNode& node)
{
  BindNodeToScope(node.id, CurrentScopeId());

  const auto* function_decl = static_cast<const FunctionDeclASTNode*>(node.decl.get());

  BindNodeToScope(function_decl->id, CurrentScopeId());

  const std::size_t parent_scope_id = CurrentScopeId();
  auto& parent_scope = CurrentScope();

  const auto duplicate_iterator = parent_scope.name_to_symbol.find(function_decl->name.value);
  if (duplicate_iterator != parent_scope.name_to_symbol.end()) {
    const auto& previous_symbol = m_Result.symbols[duplicate_iterator->second];
    ReportDuplicateDeclaration(function_decl->name, previous_symbol);
    return;
  }

  const std::size_t shadowed_symbol_id = LookupInParentScopes(function_decl->name.value);
  if (shadowed_symbol_id != -1ull) {
    ReportShadowingDeclaration(function_decl->name, m_Result.symbols[shadowed_symbol_id]);
  }

  const std::size_t function_symbol_id = m_SymbolId++;

  m_Result.symbols.push_back({
    .id = function_symbol_id,
    .name = function_decl->name.value,
    .kind = SymbolKind::FUNCTION,
    .is_exported = false,
    .declaration_scope_id = parent_scope.id,
    .declaration_node_id = function_decl->id,
    .declaration_location = ToLocation(*function_decl),
    .reference_locations = {},
    .reference_node_ids = {},
  });

  parent_scope.name_to_symbol.emplace(function_decl->name.value, function_symbol_id);
  parent_scope.declared_symbols.push_back(function_symbol_id);

  BindNodeToSymbol(node.id, function_symbol_id);
  BindNodeToSymbol(function_decl->id, function_symbol_id);

  const std::size_t function_scope_id = PushScope(
    ScopeKind::FUNCTION,
    node.id,
    parent_scope_id
  );

  if (function_decl->args != nullptr) {
    BindNodeToScope(function_decl->args->id, function_scope_id);
    function_decl->args->visit(*this);
  }

  if (node.body != nullptr) {
    BindNodeToScope(node.body->id, function_scope_id);
    node.body->visit(*this);
  }

  PopScope();
}

void Resolver::visit(const FunctionDeclASTNode &node)
{
}

void Resolver::visit(const ReturnStatementASTNode &node)
{
  BindNodeToScope(node.id, CurrentScopeId());
  node.expr->visit(*this);
}

void Resolver::visit(const FunctionCallASTNode& node)
{
  BindNodeToScope(node.id, CurrentScopeId());

  if (node.base != nullptr) {
    if (const auto* identifier_node = dynamic_cast<const IdentifierASTNode*>(node.base.get())) {
      ResolveCallTarget(*identifier_node);
    } else {
      node.base->visit(*this);
    }
  }

  for (const auto& argument_node : node.args)
  {
    if (argument_node != nullptr) {
      argument_node->visit(*this);
    }
  }
}

void Resolver::visit(const FunctionArgumentListASTNode& node)
{
  BindNodeToScope(node.id, CurrentScopeId());

  for (const auto& argument_node : node.args)
  {
    DeclareFunctionParameter(*argument_node);
  }

  if (node.args_arg != nullptr) {
    DeclareFunctionParameter(*node.args_arg);
  }

  if (node.kwargs_arg != nullptr) {
    DeclareFunctionParameter(*node.kwargs_arg);
  }
}

void Resolver::visit(const MethodCallASTNode& node)
{
  BindNodeToScope(node.id, CurrentScopeId());

  node.base_expr->visit(*this);

  for (const auto& argument_node : node.args)
  {
    argument_node->visit(*this);
  }
}

void Resolver::visit(const GetPropertyASTNode& node)
{
  BindNodeToScope(node.id, CurrentScopeId());
  node.base_expr->visit(*this);
}

void Resolver::visit(const SetPropertyASTNode& node)
{
  BindNodeToScope(node.id, CurrentScopeId());
  node.base_expr->visit(*this);
  node.RHS->visit(*this);
}

void Resolver::visit(const ClassASTNode& node)
{
  BindNodeToScope(node.id, CurrentScopeId());

  auto& parent_scope = CurrentScope();
  const std::size_t parent_scope_id = CurrentScopeId();

  const auto duplicate_iterator = parent_scope.name_to_symbol.find(node.name.value);
  if (duplicate_iterator != parent_scope.name_to_symbol.end()) {
    const auto& previous_symbol = m_Result.symbols[duplicate_iterator->second];
    ReportDuplicateDeclaration(node.name, previous_symbol);
    return;
  }

  const std::size_t shadowed_symbol_id = LookupInParentScopes(node.name.value);
  if (shadowed_symbol_id != -1ull) {
    ReportShadowingDeclaration(node.name, m_Result.symbols[shadowed_symbol_id]);
  }

  const std::size_t class_symbol_id = m_SymbolId++;

  m_Result.symbols.push_back({
    .id = class_symbol_id,
    .name = node.name.value,
    .kind = SymbolKind::TYPE,
    .is_exported = false,
    .declaration_scope_id = parent_scope.id,
    .declaration_node_id = node.id,
    .declaration_location = ToLocation(node),
    .owned_scope_id = -1ull,
    .reference_locations = {},
    .reference_node_ids = {},
  });

  parent_scope.name_to_symbol.emplace(node.name.value, class_symbol_id);
  parent_scope.declared_symbols.push_back(class_symbol_id);

  BindNodeToSymbol(node.id, class_symbol_id);

  const std::size_t class_scope_id = PushScope(
    ScopeKind::CLASS,
    node.id,
    parent_scope_id
  );

  m_Result.symbols[class_symbol_id].owned_scope_id = class_scope_id;
  m_Result.scopes[class_scope_id].owner_symbol_id = class_symbol_id;

  for (const auto& member_function_node : node.member_functions)
  {
    member_function_node->visit(*this);
  }

  PopScope();
}

void Resolver::visit(const ExportASTNode& node)
{
  BindNodeToScope(node.id, CurrentScopeId());

  if (!EnsureModuleScope(node, "Export")) { return; }

  for (const auto& variable_node : node.variables)
  {
    const auto* identifier_node = static_cast<const IdentifierASTNode*>(variable_node.get());
    ResolveExportTarget(*identifier_node);
  }
}

void Resolver::visit(const ImportASTNode& node)
{
  BindNodeToScope(node.id, CurrentScopeId());

  if (!EnsureModuleScope(node, "Import")) { return; }

  node.module->visit(*this);

  for (const auto& identifier_token : node.identifiers)
  {
    DeclareImportedSymbol(identifier_token, node.id);
  }
}

void Resolver::visit(const IndexASTNode &node)
{
}

void Resolver::visit(const StatementIndexASTNode &node)
{
}

void Resolver::visit(const KeyParamExpressionASTNode& node)
{
  BindNodeToScope(node.id, CurrentScopeId());
  node.expression->visit(*this);
}

void Resolver::ReportDuplicateDeclaration(const Token& name_token, const Symbol& prev_symbol)
{
  m_Result.diagnostics.push_back(Diagnostic{
    .severity = DiagnosticSeverity::ERROR,
    .code = DiagnosticCode::DUPLICATE_DECLARATION,
    .message = std::format("Name {} is already declared at {}:{}", name_token.value, prev_symbol.declaration_location.range.start.line, prev_symbol.declaration_location.range.start.character),
    .location = ToLocation(name_token),
  });
}

void Resolver::ReportShadowingDeclaration(const Token& name_token, const Symbol& prev_symbol)
{
  m_Result.diagnostics.push_back(Diagnostic{
    .severity = DiagnosticSeverity::WARNING,
    .code = DiagnosticCode::SHADOWING_DECLARATION,
    .message = std::format("Name {} shadows an outer declaration at {}:{}", name_token.value, prev_symbol.declaration_location.range.start.line, prev_symbol.declaration_location.range.start.character),
    .location = ToLocation(name_token),
  });
}

void Resolver::ReportUnknownIdentifier(const Token& name_token)
{
  m_Result.diagnostics.push_back(Diagnostic{
    .severity = DiagnosticSeverity::ERROR,
    .code = DiagnosticCode::UNKNOWN_IDENTIFIER,
    .message = std::format("Unknown identifier {}", name_token.value),
    .location = ToLocation(name_token),
  });
}

void Resolver::ReportConstReassignment(const Token& name_token)
{
  m_Result.diagnostics.push_back(Diagnostic{
    .severity = DiagnosticSeverity::ERROR,
    .code = DiagnosticCode::CONST_REASSIGNMENT,
    .message = std::format("Cannot reassign constant {}", name_token.value),
    .location = ToLocation(name_token),
  });
}

bool Resolver::EnsureModuleScope(const BaseASTNode& node, const std::string_view construct_name)
{
  if (CurrentScope().kind == ScopeKind::MODULE) {
    return true;
  }

  m_Result.diagnostics.push_back(Diagnostic{
    .severity = DiagnosticSeverity::ERROR,
    .code = DiagnosticCode::INVALID_MODULE_SCOPE,
    .message = std::format("{} is only allowed at module scope", construct_name),
    .location = ToLocation(node),
  });

  return false;
}

void Resolver::BindNodeToSymbol(std::size_t node_id, std::size_t symbol_id)
{
  m_Result.node_to_symbol[node_id] = symbol_id;
}

void Resolver::BindNodeToScope(std::size_t node_id, std::size_t scope_id)
{
  m_Result.node_to_scope[node_id] = scope_id;
}

void Resolver::ResolveAssignmentTarget(const IdentifierASTNode& node)
{
  BindNodeToScope(node.id, CurrentScopeId());

  const std::size_t symbol_id = LookupInCurrentOrParentScopes(node.token.value);
  if (symbol_id == -1ull) {
    ReportUnknownIdentifier(node.token);
    return;
  }

  auto& symbol = m_Result.symbols[symbol_id];

  if (symbol.kind == SymbolKind::CONSTANT) {
    ReportConstReassignment(node.token);
  }

  BindNodeToSymbol(node.id, symbol_id);

  symbol.reference_node_ids.push_back(node.id);
  symbol.reference_locations.push_back(ToLocation(node));

  m_Result.symbol_references.push_back(SymbolReference{
    .node_id = node.id,
    .symbol_id = symbol_id,
    .scope_id = CurrentScopeId(),
    .location = ToLocation(node),
    .kind = ReferenceKind::WRITE,
  });
}

void Resolver::ResolveCallTarget(const IdentifierASTNode& node)
{
  BindNodeToScope(node.id, CurrentScopeId());

  const std::size_t symbol_id = LookupInCurrentOrParentScopes(node.token.value);
  if (symbol_id == -1ull) {
    ReportUnknownIdentifier(node.token);
    return;
  }

  auto& symbol = m_Result.symbols[symbol_id];

  BindNodeToSymbol(node.id, symbol_id);

  symbol.reference_node_ids.push_back(node.id);
  symbol.reference_locations.push_back(ToLocation(node));

  m_Result.symbol_references.push_back(SymbolReference{
    .node_id = node.id,
    .symbol_id = symbol_id,
    .scope_id = CurrentScopeId(),
    .location = ToLocation(node),
    .kind = ReferenceKind::CALL,
  });
}

void Resolver::ResolveExportTarget(const IdentifierASTNode& node)
{
  BindNodeToScope(node.id, CurrentScopeId());

  const std::size_t symbol_id = LookupInCurrentOrParentScopes(node.token.value);
  if (symbol_id == -1ull) {
    ReportUnknownIdentifier(node.token);
    return;
  }

  auto& symbol = m_Result.symbols[symbol_id];

  symbol.is_exported = true;

  BindNodeToSymbol(node.id, symbol_id);

  symbol.reference_node_ids.push_back(node.id);
  symbol.reference_locations.push_back(ToLocation(node));

  m_Result.symbol_references.push_back({
    .node_id = node.id,
    .symbol_id = symbol_id,
    .scope_id = CurrentScopeId(),
    .location = ToLocation(node),
    .kind = ReferenceKind::EXPORT,
  });
}

void Resolver::DeclareFunctionParameter(const FunctionArgumentASTNode& node)
{
  BindNodeToScope(node.id, CurrentScopeId());

  auto& scope = CurrentScope();

  const auto duplicate_iterator = scope.name_to_symbol.find(node.name.value);
  if (duplicate_iterator != scope.name_to_symbol.end()) {
    const auto& previous_symbol = m_Result.symbols[duplicate_iterator->second];
    ReportDuplicateDeclaration(node.name, previous_symbol);
    return;
  }

  const std::size_t shadowed_symbol_id = LookupInParentScopes(node.name.value);
  if (shadowed_symbol_id != -1ull) {
    ReportShadowingDeclaration(node.name, m_Result.symbols[shadowed_symbol_id]);
  }

  const std::size_t symbol_id = m_SymbolId++;

  m_Result.symbols.push_back(Symbol{
    .id = symbol_id,
    .name = node.name.value,
    .kind = SymbolKind::PARAMETER,
    .is_exported = false,
    .declaration_scope_id = scope.id,
    .declaration_node_id = node.id,
    .declaration_location = ToLocation(node),
    .reference_locations = {},
    .reference_node_ids = {},
  });

  scope.name_to_symbol.emplace(node.name.value, symbol_id);
  scope.declared_symbols.push_back(symbol_id);

  BindNodeToSymbol(node.id, symbol_id);
}

void Resolver::DeclareImportedSymbol(const Token& name_token, const std::size_t declaration_node_id)
{
  auto& scope = CurrentScope();

  const auto duplicate_iterator = scope.name_to_symbol.find(name_token.value);
  if (duplicate_iterator != scope.name_to_symbol.end()) {
    const auto& previous_symbol = m_Result.symbols[duplicate_iterator->second];
    ReportDuplicateDeclaration(name_token, previous_symbol);
    return;
  }

  const std::size_t shadowed_symbol_id = LookupInParentScopes(name_token.value);
  if (shadowed_symbol_id != -1ull) {
    ReportShadowingDeclaration(name_token, m_Result.symbols[shadowed_symbol_id]);
  }

  const std::size_t symbol_id = m_SymbolId++;

  m_Result.symbols.push_back({
    .id = symbol_id,
    .name = name_token.value,
    .kind = SymbolKind::IMPORT,
    .is_exported = false,
    .declaration_scope_id = scope.id,
    .declaration_node_id = declaration_node_id,
    .declaration_location = ToLocation(name_token),
    .owned_scope_id = -1ull,
    .reference_locations = {},
    .reference_node_ids = {},
  });

  scope.name_to_symbol.emplace(name_token.value, symbol_id);
  scope.declared_symbols.push_back(symbol_id);
}

std::size_t Resolver::DeclareBuiltinSymbol(const std::string_view name, const SymbolKind kind)
{
  auto& scope = CurrentScope();

  const auto iterator = scope.name_to_symbol.find(std::string(name));
  if (iterator != scope.name_to_symbol.end()) {
    return iterator->second;
  }

  const std::size_t symbol_id = m_SymbolId++;

  m_Result.symbols.push_back(Symbol{
    .id = symbol_id,
    .name = std::string(name),
    .kind = kind,
    .is_exported = false,
    .is_builtin = true,
    .declaration_scope_id = scope.id,
    .declaration_node_id = static_cast<std::size_t>(-1),
    .declaration_location = {},
    .owned_scope_id = static_cast<std::size_t>(-1),
    .reference_locations = {},
    .reference_node_ids = {},
  });

  scope.name_to_symbol.emplace(std::string(name), symbol_id);
  scope.declared_symbols.push_back(symbol_id);

  return symbol_id;
}

std::size_t Resolver::LookupInParentScopes(std::string_view name) const
{
  if (m_ScopeStack.empty()) {
    return -1ull;
  }

  std::size_t current_parent_scope_id = CurrentScope().parent_scope_id;

  while (current_parent_scope_id != -1ull) {
    const auto& scope = m_Result.scopes[current_parent_scope_id];

    const auto iterator = scope.name_to_symbol.find(std::string(name));
    if (iterator != scope.name_to_symbol.end()) {
      return iterator->second;
    }

    current_parent_scope_id = scope.parent_scope_id;
  }

  return -1ull;
}

std::size_t Resolver::LookupInCurrentOrParentScopes(std::string_view name) const
{
  if (m_ScopeStack.empty()) {
    return -1ull;
  }

  std::size_t scope_id = CurrentScopeId();

  while (scope_id != -1ull) {
    const auto& scope = m_Result.scopes[scope_id];

    const auto iterator = scope.name_to_symbol.find(std::string(name));
    if (iterator != scope.name_to_symbol.end()) {
      return iterator->second;
    }

    scope_id = scope.parent_scope_id;
  }

  return -1ull;
}

std::size_t Resolver::PushScope(
  const ScopeKind kind,
  const std::size_t node_id,
  const std::size_t parent_scope_id
)
{
  const std::size_t scope_id = m_ScopeId++;

  m_Result.scopes.push_back(Scope{
    .id = scope_id,
    .kind = kind,
    .parent_scope_id = parent_scope_id,
    .node_id = node_id,
    .declared_symbols = {},
    .name_to_symbol = {},
  });

  m_ScopeStack.push_back(scope_id);
  return scope_id;
}

void Resolver::PopScope()
{
  if (!m_ScopeStack.empty()) {
    m_ScopeStack.pop_back();
  }
}

Scope& Resolver::CurrentScope()
{
  return m_Result.scopes[m_ScopeStack.back()];
}

const Scope& Resolver::CurrentScope() const
{
  return m_Result.scopes[m_ScopeStack.back()];
}

std::size_t Resolver::CurrentScopeId() const
{
  return CurrentScope().id;
}

void Resolver::SeedBuiltins()
{
  DeclareBuiltinSymbol("print", SymbolKind::FUNCTION);
  DeclareBuiltinSymbol("input", SymbolKind::FUNCTION);
  DeclareBuiltinSymbol(IntegerTypeObject->name, SymbolKind::TYPE);
  DeclareBuiltinSymbol(FloatTypeObject->name, SymbolKind::TYPE);
  DeclareBuiltinSymbol(ArrayTypeObject->name, SymbolKind::TYPE);
  DeclareBuiltinSymbol(BooleanTypeObject->name, SymbolKind::TYPE);
  DeclareBuiltinSymbol(StringTypeObject->name, SymbolKind::TYPE);
  DeclareBuiltinSymbol(TypeObjectTypeObject->name, SymbolKind::TYPE);
  DeclareBuiltinSymbol(DictTypeObject->name, SymbolKind::TYPE);
  DeclareBuiltinSymbol(FunctionTypeObject->name, SymbolKind::TYPE);
  DeclareBuiltinSymbol(SizeIteratorTypeObject->name, SymbolKind::TYPE);
}

Location Resolver::ToLocation(const BaseASTNode &ast) const
{
  return Location{ .file_id = m_FileId, .range = ast.range };
}

Location Resolver::ToLocation(const Token &node) const
{
  return Location{ .file_id = m_FileId, .range = node.range };
}

std::string to_string(const DiagnosticSeverity value)
{
  switch (value) {
    case DiagnosticSeverity::INFO: return "INFO";
    case DiagnosticSeverity::WARNING: return "WARNING";
    case DiagnosticSeverity::ERROR: return "ERROR";
  }
  return "ERROR";
}

std::string to_string(const DiagnosticCode value) {
  switch (value) {
    case DiagnosticCode::UNKNOWN_IDENTIFIER: return "UNKNOWN_IDENTIFIER";
    case DiagnosticCode::DUPLICATE_DECLARATION: return "DUPLICATE_DECLARATION";
    case DiagnosticCode::SHADOWING_DECLARATION: return "SHADOWING_DECLARATION";
    case DiagnosticCode::CONST_REASSIGNMENT: return "CONST_REASSIGNMENT";
    case DiagnosticCode::INVALID_RETURN: return "INVALID_RETURN";
    case DiagnosticCode::INVALID_BREAK: return "INVALID_BREAK";
    case DiagnosticCode::INVALID_CONTINUE: return "INVALID_CONTINUE";
    case DiagnosticCode::INVALID_MODULE_SCOPE: return "INVALID_MODULE_SCOPE";
    case DiagnosticCode::USE_BEFORE_DECLARATION: return "USE_BEFORE_DECLARATION";
    case DiagnosticCode::USE_BEFORE_INITIALIZATION: return "USE_BEFORE_INITIALIZATION";
    case DiagnosticCode::NOT_CALLABLE: return "NOT_CALLABLE";
    case DiagnosticCode::ARITY_MISMATCH: return "ARITY_MISMATCH";
    case DiagnosticCode::INTERNAL_RESOLVER_ERROR: return "INTERNAL_RESOLVER_ERROR";
  }
  return "INTERNAL_RESOLVER_ERROR";
}

std::string to_string(const SymbolKind value) {
  switch (value) {
    case SymbolKind::CONSTANT: return "CONSTANT";
    case SymbolKind::MUTABLE: return "MUTABLE";
    case SymbolKind::FUNCTION: return "FUNCTION";
    case SymbolKind::IMPORT: return "IMPORT";
    case SymbolKind::PARAMETER: return "PARAMETER";
    case SymbolKind::TYPE: return "TYPE";
  }
  return "TYPE";
}

std::string to_string(const ScopeKind value) {
  switch (value) {
    case ScopeKind::GLOBAL: return "GLOBAL";
    case ScopeKind::MODULE: return "MODULE";
    case ScopeKind::FUNCTION: return "FUNCTION";
    case ScopeKind::CLASS: return "CLASS";
    case ScopeKind::BLOCK: return "BLOCK";
  }
  return "BLOCK";
}

std::string to_string(const ReferenceKind value) {
  switch (value) {
    case ReferenceKind::READ: return "READ";
    case ReferenceKind::WRITE: return "WRITE";
    case ReferenceKind::CALL: return "CALL";
    case ReferenceKind::IMPORT: return "IMPORT";
    case ReferenceKind::EXPORT: return "EXPORT";
  }
  return "READ";
}

}
