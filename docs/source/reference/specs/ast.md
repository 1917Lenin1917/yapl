# AST Node

:::{cpp:namespace} yapl
:::

:::{cpp:type} ASTPtr = std::unique_ptr<BaseASTNode> 
:::

## Base node

### BaseASTNode

:::{cpp:class} BaseASTNode
Base class for all AST nodes.

**Responsibilities**

- Stores `id` and `location`.

**Invariants**

- `id` is unique within a single AST tree.
- `location` spans the syntactic construct represented by the node.
:::

:::{cpp:member} std::size_t BaseASTNode::id
Node identifier.
:::

:::{cpp:member} SourceLocation BaseASTNode::location
Source range for this node.
:::

---

## Literals

### IntegerASTNode

:::{cpp:class} IntegerASTNode
Integer literal expression node.

**Fields**

- `value` is parsed from the associated token.

**Notes**

- Parsing uses `std::stoi(token.value)`.
- If `token.value` is not a valid integer, construction throws.
:::

:::{cpp:member} int IntegerASTNode::value
Decoded integer value.
:::

---

### FloatASTNode

:::{cpp:class} FloatASTNode
Floating-point literal expression node.

**Notes**

- Parsing uses `std::stof(token.value)`.
- If `token.value` is not a valid float, construction throws.
:::

:::{cpp:member} float FloatASTNode::value
Decoded floating-point value.
:::

---

### BooleanASTNode

:::{cpp:class} BooleanASTNode
Boolean literal expression node.

**Semantics**

- `value` is `true` if `token.value == "true"`.
:::

:::{cpp:member} bool BooleanASTNode::value
Decoded boolean value.
:::

---

### StringASTNode

:::{cpp:class} StringASTNode
String literal expression node.

**Semantics**

- `value` stores the token payload as-is.
- Escapes and quoting rules are defined in the lexer spec.
:::

:::{cpp:member} std::string StringASTNode::value
String payload.
:::

---

## Names and access

### IdentifierASTNode

:::{cpp:class} IdentifierASTNode
Identifier expression node.

Stores the full `Token` to preserve spelling, span, and any lexer metadata.
:::

:::{cpp:member} Token IdentifierASTNode::token
Identifier token.
:::

---

### GetPropertyASTNode

:::{cpp:class} GetPropertyASTNode
Property access expression.

Represents `base_expr.name`.
:::

:::{cpp:member} ASTPtr GetPropertyASTNode::base_expr
Base expression.
:::

:::{cpp:member} Token GetPropertyASTNode::name
Property name token.
:::

---

### SetPropertyASTNode

:::{cpp:class} SetPropertyASTNode
Property assignment expression.

Represents `base_expr.name = RHS`.
:::

:::{cpp:member} ASTPtr SetPropertyASTNode::base_expr
Base expression.
:::

:::{cpp:member} Token SetPropertyASTNode::name
Property name token.
:::

:::{cpp:member} ASTPtr SetPropertyASTNode::RHS
Assigned expression.
:::

---

## Calls

### FunctionCallASTNode

:::{cpp:class} FunctionCallASTNode
Function call expression.

Represents `base(args...)`.
:::

:::{cpp:member} ASTPtr FunctionCallASTNode::base
Callee expression.
:::

:::{cpp:member} std::vector<ASTPtr> FunctionCallASTNode::args
Argument expressions in source order.
:::

---

### MethodCallASTNode

:::{cpp:class} MethodCallASTNode
Method call expression.

Represents `base_expr.name(args...)`.
:::

:::{cpp:member} ASTPtr MethodCallASTNode::base_expr
Receiver expression.
:::

:::{cpp:member} Token MethodCallASTNode::name
Method name token.
:::

:::{cpp:member} std::vector<ASTPtr> MethodCallASTNode::args
Argument expressions in source order.
:::

