# Token

:::{cpp:enum-class} Token
:::

## Literals

:::{cpp:enumerator} Token::INTEGER
Integer literal.
Represents a whole-number numeric literal, for example `123`.
:::

:::{cpp:enumerator} Token::FLOAT
Floating-point literal.
Represents a decimal numeric literal, for example `3.14`.
:::

:::{cpp:enumerator} Token::STRING
String literal.
Represents a quoted string value, for example `'foo'` or `"bar"`.
:::

:::{cpp:enumerator} Token::FSTRING
Formatted string literal.
Represents a string with embedded interpolation segments, for example `` `Hello, {name}!` ``.
:::

:::{cpp:enumerator} Token::BOOL
Boolean literal.
Represents the `true` and `false` literal values.
:::

## Operators

(token-plus)=
:::{cpp:enumerator} Token::PLUS
Addition operator.

Lexeme: `+`
:::

:::{cpp:enumerator} Token::PLUSEQ
Add-assign operator.

Lexeme: `+=`
:::

(token-minus)=
:::{cpp:enumerator} Token::MINUS
Subtraction operator.

Lexeme: `-`
:::

:::{cpp:enumerator} Token::MINUSEQ
Subtract-assign operator.

Lexeme: `-=`
:::

:::{cpp:enumerator} Token::TIMES
Multiplication operator.

Lexeme: `*`
:::

:::{cpp:enumerator} Token::TIMESEQ
Multiply-assign operator.

Lexeme: `*=`
:::

:::{cpp:enumerator} Token::MOD
Modulo operator.

Lexeme: `%`
:::

:::{cpp:enumerator} Token::MODEQ
Modulo-assign operator.

Lexeme: `%=`
:::

:::{cpp:enumerator} Token::SLASH
Division operator.

Lexeme: `/`
:::

:::{cpp:enumerator} Token::SLASHEQ
Divide-assign operator.

Lexeme: `/=`
:::

:::{cpp:enumerator} Token::PERIOD
Member access.

Lexeme: `.`
:::

(token-not)=
:::{cpp:enumerator} Token::NOT
Logical negation operator.

Lexeme: `!`
:::

:::{cpp:enumerator} Token::ASSIGN
Assignment operator.

Lexeme: `=`
:::

:::{cpp:enumerator} Token::EQ
Equality comparison operator.

Lexeme: `==`
:::

:::{cpp:enumerator} Token::NEQ
Inequality comparison operator.

Lexeme: `!=`
:::

:::{cpp:enumerator} Token::LT
Less-than comparison operator.

Lexeme: `<`
:::

:::{cpp:enumerator} Token::LQ
Less-than-or-equal comparison operator.

Lexeme: `<=`
:::

:::{cpp:enumerator} Token::GT
Greater-than comparison operator.

Lexeme: `>`
:::

:::{cpp:enumerator} Token::GQ
Greater-than-or-equal comparison operator.

Lexeme: `>=`
:::

:::{cpp:enumerator} Token::ARROW
Arrow operator.

Lexeme: `=>`
:::

:::{cpp:enumerator} Token::OR
Logical OR operator.

Keyword form: `or`
:::

:::{cpp:enumerator} Token::AND
Logical AND operator.

Keyword form: `and`
:::

## Separators

:::{cpp:enumerator} Token::LPAREN
Left parenthesis.

Lexeme: `(`
:::

:::{cpp:enumerator} Token::RPAREN
Right parenthesis.

Lexeme: `)`
:::

:::{cpp:enumerator} Token::LBRACK
Left brace.

Lexeme: `{`
:::

:::{cpp:enumerator} Token::RBRACK
Right brace.

Lexeme: `}`
:::

:::{cpp:enumerator} Token::LSQBRACK
Left square bracket.

Lexeme: `[`
:::

:::{cpp:enumerator} Token::RSQBRACK
Right square bracket.

Lexeme: `]`
:::

:::{cpp:enumerator} Token::SEMICOLON
Statement terminator.

Lexeme: `;`
:::

:::{cpp:enumerator} Token::COLON
Colon separator.

Lexeme: `:`
:::

:::{cpp:enumerator} Token::COMMA
Comma separator.

Lexeme: `,`
:::

## Keywords

:::{cpp:enumerator} Token::IF
Introduces a conditional branch.

Keyword form: `if`
:::

:::{cpp:enumerator} Token::ELSE
Introduces the fallback branch of a conditional.

Keyword form: `else`
:::

:::{cpp:enumerator} Token::FOR
Introduces a `for` loop.

Keyword form: `for`
:::

:::{cpp:enumerator} Token::FN
Introduces a function declaration.

Keyword form: `fn`
:::

:::{cpp:enumerator} Token::VAR
Introduces a mutable variable declaration.

Keyword form: `var`
:::

:::{warning}
[Token::VAR](Token::VAR) is deprecated and will be removed soon. Use [Token::LET](Token::LET) instead.
:::

:::{cpp:enumerator} Token::LET
Introduces a local binding.

Keyword form: `let`
:::

:::{cpp:enumerator} Token::CONST
Introduces an immutable constant declaration.

Keyword form: `const`
:::

:::{cpp:enumerator} Token::RETURN
Returns from the current function.

Keyword form: `return`
:::

:::{cpp:enumerator} Token::WHILE
Introduces a `while` loop.

Keyword form: `while`
:::

:::{cpp:enumerator} Token::CLASS
Introduces a class declaration.

Keyword form: `class`
:::

:::{cpp:enumerator} Token::IMPORT
Introduces an import statement.

Keyword form: `import`
:::

:::{cpp:enumerator} Token::EXPORT
Marks a symbol for export.

Keyword form: `export`
:::

:::{cpp:enumerator} Token::FROM
Used in import/export source clauses.

Keyword form: `from`
:::

## Special

:::{cpp:enumerator} Token::IDENTIFIER
User-defined identifier.
Represents names for variables, functions, types, and other symbols, for example `name`, `Logger`.
:::

:::{cpp:enumerator} Token::TT_EOF
End-of-file marker.
Produced by the lexer when the input stream is exhausted.
:::

