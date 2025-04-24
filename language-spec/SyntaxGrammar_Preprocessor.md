```
from SynaxGrammar import statement ; 

directive   → "#" include | define | if | ifdef | ifndef | pragma | print EOL ;
include     → "include" macro ;
define      → ( "define" IDENTIFIER macro ) | ( "undef" IDENTIFIER ) ;
if          → "if" macro EOL statement* EOL elses ;
ifdef       → "ifdef" IDENTIFIER EOL statement* EOL elses ;
ifndef      → "ifndef" IDENTIFIER EOL statement* EOL elses ;
  elses     → ( ( ( "elif" macro ) | "else" ) EOL statement* EOL )* "endif" ;
print       → "print" IDENTIFIER? ;
macro       → expression ;

expression  → logic ;  
logic       → equality ( ( "&" | "and" | "|" | "or" ) equality )* ;  
equality    → relational ( ( "!=" | "==" ) relational )* ;  
relational  → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;  
term        → factor ( ( "+" | "-" ) factor )* ;  
factor      → exponent ( ( "*" | "/" | "%" ) exponent )* ;  
exponent    → unary ( "^" unary )* ;  
unary       → ( "!" | "not" | "+" | "-" )? primary ;  
primary     → LITERAL | IDENTIFIER | "(" expression ")" ;  

PREDEFINED MACROS
__MAIN__    = Defined when compiling the original file given to mxslc.compile_file(...).
__INCLUDE__ = Defined when compiling a file from an #include directive.
__TARGET__  = Name of the renderer specified when calling mxslc.compile_file(...), or undefined if not specified.
__VERSION__ = Version of MaterialX used during compilation, taken from the MaterialX Python API.
  
LEGEND  
* = 0 or more  
+ = 1 or more  
? = 0 or 1  
{x,y} = between x and y (inclusive)  
```  
  