//
// Created by lenin on 01.03.2026.
//

#pragma once
#include <unordered_map>
#include <sstream>

#include "ASTNode.hpp"

namespace yapl {

struct Position
{
  std::size_t line;
  std::size_t character;
};

struct Location
{
  std::size_t file_id;
  Position start;
  Position end;
};

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

  std::size_t parent_scope_id; // -1 if no parent
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
};

class Resolver : public Visitor
{
public:
  explicit Resolver();

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

  void visit(const IfElseExpressionASTNode &node) override;
  void visit(const ScopeASTNode &node) override;
  void visit(const ForLoopASTNode &node) override;
  void visit(const WhileLoopASTNode &node) override;
  void visit(const StatementASTNode &node) override;
  void visit(const FunctionASTNode &node) override;
  void visit(const ReturnStatementASTNode &node) override;
  void visit(const FunctionCallASTNode& node) override;
  void visit(const FunctionArgumentListASTNode &node) override;
  void visit(const MethodCallASTNode &node) override;
  void visit(const GetPropertyASTNode &node) override;
  void visit(const SetPropertyASTNode &node) override;
  void visit(const ClassASTNode &node) override;
  void visit(const ExportASTNode &node) override;
  void visit(const ImportASTNode &node) override;

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

private:
  std::vector<std::size_t> m_ScopeStack;

  ResolutionResult m_Result;

  std::size_t m_ScopeId = 0;
  std::size_t m_SymbolId = 0;
};


inline std::string to_string(DiagnosticSeverity value) {
    switch (value) {
        case DiagnosticSeverity::INFO: return "INFO";
        case DiagnosticSeverity::WARNING: return "WARNING";
        case DiagnosticSeverity::ERROR: return "ERROR";
    }
    return "ERROR";
}

inline std::string to_string(DiagnosticCode value) {
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

inline std::string to_string(SymbolKind value) {
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

inline std::string to_string(ScopeKind value) {
    switch (value) {
        case ScopeKind::GLOBAL: return "GLOBAL";
        case ScopeKind::MODULE: return "MODULE";
        case ScopeKind::FUNCTION: return "FUNCTION";
        case ScopeKind::CLASS: return "CLASS";
        case ScopeKind::BLOCK: return "BLOCK";
    }
    return "BLOCK";
}

inline std::string to_string(ReferenceKind value) {
    switch (value) {
        case ReferenceKind::READ: return "READ";
        case ReferenceKind::WRITE: return "WRITE";
        case ReferenceKind::CALL: return "CALL";
        case ReferenceKind::IMPORT: return "IMPORT";
        case ReferenceKind::EXPORT: return "EXPORT";
    }
    return "READ";
}

inline void write_escaped_json_string(std::ostream& output, std::string_view value) {
    output.put('"');
    for (char character : value) {
        switch (character) {
            case '"': output << "\\\""; break;
            case '\\': output << "\\\\"; break;
            case '\b': output << "\\b"; break;
            case '\f': output << "\\f"; break;
            case '\n': output << "\\n"; break;
            case '\r': output << "\\r"; break;
            case '\t': output << "\\t"; break;
            default:
                if (static_cast<unsigned char>(character) < 0x20) {
                    static const char* hex = "0123456789abcdef";
                    output << "\\u00";
                    output.put(hex[(character >> 4) & 0x0f]);
                    output.put(hex[character & 0x0f]);
                } else {
                    output.put(character);
                }
                break;
        }
    }
    output.put('"');
}

inline void write_json(std::ostream& output, const Position& position) {
    output << "{";
    output << "\"line\":" << position.line << ",";
    output << "\"character\":" << position.character;
    output << "}";
}

inline void write_json(std::ostream& output, const Location& declaration) {
    output << "{";
    output << "\"file_id\":" << declaration.file_id << ",";
    output << "\"range\":";
    output << "{";
    output << "\"start\":";
    write_json(output, declaration.start);
    output << ",";
    output << "\"end\":";
    write_json(output, declaration.end);
    output << "}";
    output << "}";
}

inline void write_json(std::ostream& output, const Diagnostic& diagnostic) {
    output << "{";
    output << "\"severity\":";
    write_escaped_json_string(output, to_string(diagnostic.severity));
    output << ",";
    output << "\"code\":";
    write_escaped_json_string(output, to_string(diagnostic.code));
    output << ",";
    output << "\"message\":";
    write_escaped_json_string(output, diagnostic.message);
    output << ",";
    output << "\"location\":";
    write_json(output, diagnostic.location);
    output << "}";
}

inline void write_json(std::ostream& output, const Symbol& symbol) {
    output << "{";
    output << "\"id\":" << symbol.id << ",";
    output << "\"name\":";
    write_escaped_json_string(output, symbol.name);
    output << ",";
    output << "\"kind\":";
    write_escaped_json_string(output, to_string(symbol.kind));
    output << ",";
    output << "\"is_exported\":" << (symbol.is_exported ? "true" : "false") << ",";
    output << "\"is_builtin\":" << (symbol.is_builtin ? "true" : "false") << ",";
    output << "\"declaration_scope_id\":" << symbol.declaration_scope_id << ",";
    output << "\"owned_scope_id\":" << symbol.owned_scope_id << ",";
    output << "\"declaration\":";
    write_json(output, symbol.declaration_location);
    output << "}";
}

inline void write_json(std::ostream& output, const Scope& scope) {
    output << "{";
    output << "\"id\":" << scope.id << ",";
    output << "\"kind\":";
    write_escaped_json_string(output, to_string(scope.kind));
    output << ",";
    output << "\"parent_scope_id\":" << scope.parent_scope_id << ",";
    output << "\"owner_symbol_id\":" << scope.owner_symbol_id << ",";
    // output << "\"range\":";
    // write_json(output, scope.);
    // output << ",";
    output << "\"declared_symbols\":[";
    for (std::size_t i = 0; i < scope.declared_symbols.size(); ++i) {
        if (i != 0) {
            output << ",";
        }
        output << scope.declared_symbols[i];
    }
    output << "]";
    output << "}";
}

inline void write_json(std::ostream& output, const SymbolReference& reference) {
    output << "{";
    output << "\"symbol_id\":" << reference.symbol_id << ",";
    output << "\"kind\":";
    write_escaped_json_string(output, to_string(reference.kind));
    output << ",";
    output << "\"file_id\":" << reference.location.file_id << ",";
    output << "\"range\":";
    write_json(output, reference.location);
    output << "}";
}

// inline void write_json(std::ostream& output, const Occurrence& occurrence) {
//     output << "{";
//     output << "\"symbol_id\":" << occurrence.symbol_id << ",";
//     output << "\"role\":";
//     write_escaped_json_string(output, to_string(occurrence.role));
//     output << ",";
//     output << "\"file_id\":" << occurrence.file_id << ",";
//     output << "\"range\":";
//     write_json(output, occurrence.range);
//     output << "}";
// }
//
template <typename Item>
inline void write_json_array(std::ostream& output, const std::vector<Item>& items) {
    output << "[";
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (i != 0) {
            output << ",";
        }
        write_json(output, items[i]);
    }
    output << "]";
}

inline void serialize_json(const ResolutionResult& resolver_output, std::ostream& output) {
    output << "{";
    output << "\"version\":" << resolver_output.version << ",";
    output << "\"file_id\":" << resolver_output.file_id << ",";
    // output << "\"uri\":";
    // write_escaped_json_string(output, resolver_output.file_uri);
    // output << ",";
    output << "\"diagnostics\":";
    write_json_array(output, resolver_output.diagnostics);
    output << ",";
    output << "\"symbols\":";
    write_json_array(output, resolver_output.symbols);
    output << ",";
    output << "\"scopes\":";
    write_json_array(output, resolver_output.scopes);
    output << ",";
    output << "\"references\":";
    write_json_array(output, resolver_output.symbol_references);
    // output << ",";
    // output << "\"occurrences\":";
    // write_json_array(output, resolver_output.occurrences);
    output << "}";
}

inline std::string to_json(const ResolutionResult& resolver_output) {
    std::ostringstream output;
    serialize_json(resolver_output, output);
    return output.str();
}

}
