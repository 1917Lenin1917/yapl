# Grammar

## Operators

:::{code}
unary_op
    : +  
    | -  
    | !  
    ;
:::

:::{code}
binary_op
    : +
    | -
    | *
    | /
    | ==
    | !=
    | <
    | >
    | <=
    | >=
    ;
:::

## Literals

:::{code}
literal
    : INTEGER
    | FLOAT
    | STRING
    | FSTRING
    | BOOL
    ;
:::


## Identifiers

:::{code}
identifier: [a-zA-Z_][a-zA-Z0-9_]*;
:::


## TODO

:::{code}

array
    : "[" "]"
    | "[" expression ("," expression)* "]"
    ;

dict
   : "{" "}"
   | "{" expression ":" expression ("," expression ":" expression )* "}"
   ;
:::

:::{code}

function_call
    : identifier "(" argument_list? ")"
    ;

method_call
    : identifier "." identifier "(" argument_list? ")"
    ;

argument_list
    : positional_argument ("," positional_argument)* ("," named_argument ("," named_argument)*)?
    | named_argument ("," named_argument)*
    ;

positional_argument
    : expression
    ;

named_argument
    : identifier '=' expression
    ;

index_get: identifier "[" expression "]";



property_get: identifier "." identifier;

:::


## Expressions
:::{code}

unary_expression
    : unary_op primary_expression
    | unary_op paren_expression
    ;

paren_expression: "(" expression ")";

primary_expression
    : literal
    | function_call
    | index_get
    | method_call
    | array
    | dict
    | paren_expression
    | unary_expression
    | property_get
    ;

expression: primary_expression (binary_op primary_expression)*;

FIXME: this is wrong, we dont have operations like += etc
statement
    : identifier   "=" expression
    | index_get    "=" expression
    | property_get "=" expression
    | "return" expression?
    ;
:::

:::{code}
var_declaration
    : ("let" | "const") identifier ("=" expression ("," identifier ("=" expression)?)*)?
    ;

function_declaration
    : "fn" identifier "(" parameter_list? ")" scope
    ;

parameter_list: parameter ("," parameter)*;

parameter
    : positional_parameter
    | keyword_parameter
    ;

positional_parameter: identifier;
keyword_parameter: "=" identifier;
    
    
class_declaration: "class" identifier "{" class_members* "}"  ;
class_members: function_declaration+;

scope: "{" statement* "}"

for_loop
    : "for" variable_declaration ";" expression ";" statement ";" scope
    | "for" identifier ":" expression scope
    ;

while_loop: "while" expression scope;
:::

## Import & Export

:::{code}
import: "import" "{" identifier+ "}" "from" STRING;

export
    : "export" variable_declaration
    | "export" function_declaration
    | "export" class_declaration
    | "export" "{" identifier+ "}"
    ;
:::
