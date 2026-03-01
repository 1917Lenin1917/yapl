# `ResolverOutput`

- `version`
- `file_id`
- `uri`
- `diagnostics`
- `symbols`
- `scopes`
- `references`
- `occurrences`

# `diagnostics`

Список диагностик для файла.

Каждый элемент:

- `severity`
- `code`
- `message`
- `range`

# `range`

Диапазон в файле.

- `start`
    - `line`
    - `character`
- `end`
    - `line`
    - `character`

# `symbols`

Список всех символов, известных в этом snapshot.

Каждый элемент:

- `id`
- `name`
- `kind`
- `is_exported`
- `is_builtin`
- `declaration_scope_id`
- `owned_scope_id`
- `declaration`
    - `file_id`
    - `range`

# `scopes`

Список всех scope в файле.

Каждый элемент:

- `id`
- `kind`
- `parent_scope_id`
- `owner_symbol_id`
- `range`
- `declared_symbols`

# `references`

Список всех использований символов.

Каждый элемент:

- `symbol_id`
- `kind`
- `file_id`
- `range`

# `occurrences`

Позиционный индекс для быстрого поиска symbol по курсору.

Каждый элемент:

- `symbol_id`
- `role`
- `file_id`
- `range`

`role`:

- `declaration`
- `reference`

# Что для чего нужно

## `diagnostics`

Используется для:

- `publishDiagnostics`

## `symbols`

Используется для:

- `goto definition`
- `hover`
- `document symbols`
- `workspace symbols`
- `completion`
- `method completion`

## `scopes`

Используется для:

- completion по текущей позиции
- поиск видимых имён
- анализ вложенности
- поднятие по parent scope

## `references`

Используется для:

- `find references`
- `rename`
- `document highlight`

## `occurrences`

Используется для:

- найти symbol под курсором
- быстро понять, это declaration или usage
- база для:
    - `goto definition`
    - `hover`
    - `rename`
    - `references`

# Важные enum-значения

## `DiagnosticSeverity`

- `INFO`
- `WARNING`
- `ERROR`

## `DiagnosticCode`

- `UNKNOWN_IDENTIFIER`
- `DUPLICATE_DECLARATION`
- `SHADOWING_DECLARATION`
- `CONST_REASSIGNMENT`
- `INVALID_RETURN`
- `INVALID_BREAK`
- `INVALID_CONTINUE`
- `INVALID_MODULE_SCOPE`
- `USE_BEFORE_DECLARATION`
- `USE_BEFORE_INITIALIZATION`
- `NOT_CALLABLE`
- `ARITY_MISMATCH`
- `INTERNAL_RESOLVER_ERROR`

## `SymbolKind`

- `CONSTANT`
- `MUTABLE`
- `FUNCTION`
- `IMPORT`
- `PARAMETER`
- `TYPE`

## `ScopeKind`

- `GLOBAL`
- `MODULE`
- `FUNCTION`
- `CLASS`
- `BLOCK`

## `ReferenceKind`

- `READ`
- `WRITE`
- `CALL`
- `IMPORT`
- `EXPORT`

# Логические связи

## symbol → declaration

- `symbols[i].declaration`

## symbol → owned scope

- `symbols[i].owned_scope_id`

Это важно для:

- функций
- классов

## scope → parent scope

- `scopes[i].parent_scope_id`

## scope → declared symbols

- `scopes[i].declared_symbols`

## scope → owner symbol

- `scopes[i].owner_symbol_id`

## occurrence/reference → symbol

- `occurrences[i].symbol_id`
- `references[i].symbol_id`

# Как это будет использовать LSP

## `goto definition`

1. найти `occurrence` под курсором
2. взять `symbol_id`
3. взять `symbols[symbol_id].declaration`

## `find references`

1. найти `symbol_id` под курсором
2. отфильтровать `references` по `symbol_id`

## `rename`

1. найти `symbol_id`
2. взять:
    - declaration
    - все references
3. заменить все ranges

## `completion`

1. найти scope по позиции
2. подняться по `parent_scope_id`
3. собрать `declared_symbols`

## `method completion`

1. определить type symbol
2. взять `type_symbol.owned_scope_id`
3. взять `scopes[owned_scope_id].declared_symbols`

# Минимальная v1-версия

Если резать до самого полезного минимума, то resolver должен отдавать:

- `version`
- `file_id`
- `uri`
- `diagnostics`
- `symbols`
- `scopes`
- `references`
- `occurrences`