```  
program     → statement* EOF ;  
  
statement   → ( attribute* ) ( declaration | assignment | for_loop ) ;
  attribute → "@" IDENTIFIER STRING_LITERAL
declaration → var_decl | func_decl;  
var_decl    → TYPE IDENTIFIER "=" expression ";" ;  
func_decl   → TYPE IDENTIFIER ( "<" TYPE ( "," TYPE )* ">" )? "(" ( parameter ( "," parameter )* )? ")" "{" statement* return "}" ;  
  parameter → TYPE IDENTIFIER ( "=" expression )? ;  
  return    → "return" expression ";" ;  
assignment  → var_assign | cmp_assign ;  
var_assign  → variable "=" expression ";" ;  
cmp_assign  → variable ( "+=" | "-=" | "*=" | "/=" | "%=" | "^=" | "&=" | "|=" ) expression ";" ;  
  variable  → IDENTIFIER ( "." IDENTIFIER )?  
for_loop    → "for" "(" TYPE IDENTIFIER "=" constant ":" constant ( ":" constant )? ")" "{" statement* "}" ;  
  constant  → FLOAT_LITERAL | IDENTIFIER ; 
expr_stmt   → func_call | stdlib_call ";" ;
  
expression  → logic ;  
logic       → equality ( ( "&" | "and" | "|" | "or" ) equality )* ;  
equality    → relational ( ( "!=" | "==" ) relational )* ;  
relational  → term | ( term rel_ops term ) | ( term rel_ops term rel_ops term ) ;  
  rel_ops   → ">" | ">=" | "<" | "<="
term        → factor ( ( "+" | "-" ) factor )* ;  
factor      → exponent ( ( "*" | "/" | "%" ) exponent )* ;  
exponent    → unary ( "^" unary )* ;  
unary       → ( "!" | "not" | "+" | "-" )? property ;    
property    → primary swizzle* indexer? ;   
  swizzle   → "." ( [xyzw]{1,4} | [rgba]{1,4} ) ;  
  indexer   → "[" expression "]" ;  
primary     → LITERAL | IDENTIFIER | "(" expression ")" | cond_expr | func_call | ctor_call | node_ctor;  
cond_expr   → if_expr | switch_expr ;  
if_expr     → "if" "(" expression ")" "{" expression "}" ( "else" "{" expression "}" )? ;  
switch_expr → "switch" "(" expression ")" "{" expression ( "," expression )* "}" ;  
func_call   → IDENTIFIER ( "<" TYPE ">" )? "(" ( argument ( "," argument )* )? ")" ;  
ctor_call   → TYPE "(" ( argument ( "," argument )* )? ")" ;  
node_ctor   → "{" STRING_LITERAL "," TYPE ( ":" argument ( "," argument )* )? "}" ;
  argument  → ( IDENTIFIER "=" )? expression ;  
  
LEGEND  
* = 0 or more  
+ = 1 or more  
? = 0 or 1  
{x,y} = between x and y (inclusive)  
```  
  
