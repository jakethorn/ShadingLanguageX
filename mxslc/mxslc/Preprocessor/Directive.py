from enum import StrEnum


class Directive(StrEnum):
    DEFINE = "#define"
    UNDEF = "#undef"
    IF = "#if"
    IFDEF = "#ifdef"
    IFNDEF = "#ifndef"
    ELIF = "#elif"
    ELSE = "#else"
    ENDIF = "#endif"
    INCLUDE = "#include"
    PRAGMA = "#pragma"
    PRINT = "#print"


# access directives without qualifier
DEFINE = Directive.DEFINE
UNDEF = Directive.UNDEF
IF = Directive.IF
IFDEF = Directive.IFDEF
IFNDEF = Directive.IFNDEF
ELIF = Directive.ELIF
ELSE = Directive.ELSE
ENDIF = Directive.ENDIF
INCLUDE = Directive.INCLUDE
PRAGMA = Directive.PRAGMA
PRINT = Directive.PRINT

DIRECTIVES = [d for d in Directive]
