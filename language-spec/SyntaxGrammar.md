```  
program     → statement* EOF ;  
  
statement   → declaration | assignment | for_loop ;  
declaration → var_decl | func_decl;  
var_decl    → TYPE IDENTIFIER "=" expression ";" ;  
func_decl   → TYPE IDENTIFIER "(" ( parameter ( "," parameter )* )? ")" "{" statement* return "}" ;  
  parameter → TYPE IDENTIFIER ;  
return      → "return" expression ";" ;  
assignment  → var_assign | cmp_assign ;  
var_assign  → variable "=" expression ";" ;  
cmp_assign  → variable ( "+=" | "-=" | "*=" | "/=" | "%=" | "^=" | "&=" | "|=" ) expression ";" ;  
  variable  → IDENTIFIER ( "." IDENTIFIER )?  
for_loop    → "for" "(" TYPE IDENTIFIER "=" constant ":" constant ( ":" constant )? ")" "{" statement* "}" ;  
  constant  → LITERAL | IDENTIFIER ; 
  
expression  → logic ;  
logic       → equality ( ( "&" | "and" | "|" | "or" ) equality )* ;  
equality    → relational ( ( "!=" | "==" ) relational )* ;  
relational  → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;  
term        → factor ( ( "+" | "-" ) factor )* ;  
factor      → exponent ( ( "*" | "/" | "%" ) exponent )* ;  
exponent    → unary ( "^" unary )* ;  
unary       → ( "!" | "not" | "+" | "-" )? primary ;  
swizzle     → primary ( "." ( [xyzw]{1,4} | [rgba]{1,4} ) )? ;  
primary     → LITERAL | IDENTIFIER | "(" expression ")" | cond_expr | func_call | stdlib_call | ctor_call | node_ctor;  
cond_expr   → if_expr | switch_expr ;  
if_expr     → "if" "(" expression ")" "{" expression "}" ( "else" "{" expression "}" )? ;  
switch_expr → "switch" "(" expression ")" "{" expression ( "," expression )* "}" ;  
func_call   → IDENTIFIER "(" ( argument ( "," argument )* )? ")" ;  
stdlib_call → STDLIB_FUNC "(" ( argument ( "," argument )* )? ")" ;  
ctor_call   → TYPE "(" ( argument ( "," argument )* )? ")" ;  
node_ctor   → "{" STRING_LITERAL "," TYPE ( ":" argument ( "," argument )* )? "}" ;
  argument  → ( IDENTIFIER "=" )? expression ;  
  
LEGEND  
* = 0 or more  
+ = 1 or more  
? = 0 or 1  
{x,y} = between x and y (inclusive)  
```  
  