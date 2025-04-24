from .Argument import Argument
from .CompileError import CompileError
from .Expressions import *
from .Keyword import DATA_TYPES, Keyword
from .Parameter import Parameter
from .StandardLibrary import StandardLibrary
from .Statements import *
from .Token import Token
from .TokenReader import TokenReader
from .token_types import EOF, IDENTIFIER, FLOAT_LITERAL, INT_LITERAL, STRING_LITERAL, FILENAME_LITERAL


def parse(tokens: list[Token]) -> list[Statement]:
    return Parser(tokens).parse()


class Parser(TokenReader):
    def __init__(self, tokens: list[Token]):
        super().__init__(tokens)

    def parse(self) -> list[Statement]:
        return self.__program()

    def __program(self) -> list[Statement]:
        statements = []
        while self._peek() != EOF:
            statements.append(self.__statement())
        return statements

    def __statement(self) -> Statement:
        token = self._peek()
        if token in DATA_TYPES:
            return self.__declaration()
        if token == IDENTIFIER:
            if self._peek_next() == "(":
                expr = self.__primary() # function or standard library call
                self._match(";")
                return ExpressionStatement(expr)
            else:
                return self.__assignment()
        if token == Keyword.FOR:
            return self.__for_loop()
        raise CompileError(token.line, f"Expected return statement, data type keyword, identifier or 'for', but found '{token.lexeme}'.")

    def __declaration(self) -> Statement:
        data_type = self._match(*DATA_TYPES)
        identifier = self._match(IDENTIFIER)
        token = self._peek()
        if token == "=":
            return self.__variable_declaration(data_type, identifier)
        if token == "(":
            return self.__function_declaration(data_type, identifier)
        raise CompileError(token.line, f"Expected '=' or '(', but found '{token.lexeme}'.")

    def __variable_declaration(self, data_type: Token, identifier: Token) -> VariableDeclaration:
        self._match("=")
        right = self.__expression()
        self._match(";")
        return VariableDeclaration(data_type, identifier, right)

    def __function_declaration(self, data_type: Token, identifier: Token) -> FunctionDeclaration:
        self._match("(")
        if self._consume(")"):
            params = []
        else:
            params = [self.__parameter()]
            while self._consume(","):
                params.append(self.__parameter())
            self._match(")")
        self._match("{")
        statements = []
        while self._peek() != Keyword.RETURN:
            statements.append(self.__statement())
        self._match(Keyword.RETURN)
        return_expr = self.__expression()
        self._match(";")
        self._match("}")
        return FunctionDeclaration(data_type, identifier, params, statements, return_expr)

    def __parameter(self) -> Parameter:
        data_type = self._match(*DATA_TYPES)
        identifier = self._match(IDENTIFIER)
        return Parameter(identifier, data_type)

    def __assignment(self) -> Statement:
        identifier = self._match(IDENTIFIER)
        property = self._match(IDENTIFIER) if self._consume(".") else None
        token = self._peek()
        if token == "=":
            return self.__variable_assignment(identifier, property)
        if token in ["+=", "-=", "*=", "/=", "%=", "^=", "&=", "|="]:
            return self.__compound_assignment(identifier, property)
        raise CompileError(token.line, f"Unexpected token: '{token.lexeme}'.")

    def __variable_assignment(self, identifier: Token, property: Token) -> VariableAssignment:
        self._match("=")
        right = self.__expression()
        self._match(";")
        return VariableAssignment(identifier, property, right)

    def __compound_assignment(self, identifier: Token, property: Token) -> CompoundAssignment:
        operator = self._match("+=", "-=", "*=", "/=", "%=", "^=", "&=", "|=")
        right = self.__expression()
        self._match(";")
        return CompoundAssignment(identifier, property, operator, right)

    def __for_loop(self) -> ForLoop:
        self._match(Keyword.FOR)
        self._match("(")
        data_type = self._match(*DATA_TYPES)
        identifier = self._match(IDENTIFIER)
        self._match("=")
        literal1 = self._match(FLOAT_LITERAL, IDENTIFIER)
        self._match(":")
        literal2 = self._match(FLOAT_LITERAL, IDENTIFIER)
        if self._consume(":"):
            literal3 = self._match(FLOAT_LITERAL, IDENTIFIER)
        else:
            literal3 = None
        self._match(")")
        self._match("{")
        statements = []
        while self._peek() != "}":
            statements.append(self.__statement())
        self._match("}")
        return ForLoop(data_type, identifier, literal1, literal2, literal3, statements)

    def __expression(self) -> Expression:
        return self.__logic()

    def __logic(self) -> Expression:
        expr = self.__equality()
        while op := self._consume("&", Keyword.AND, "|", Keyword.OR):
            right = self.__equality()
            expr = LogicExpression(expr, op, right)
        return expr

    def __equality(self) -> Expression:
        expr = self.__relational()
        while op := self._consume("!=", "=="):
            right = self.__relational()
            expr = ComparisonExpression(expr, op, right)
        return expr

    def __relational(self) -> Expression:
        expr = self.__term()
        while op := self._consume(">", ">=", "<", "<="):
            right = self.__term()
            expr = ComparisonExpression(expr, op, right)
        return expr

    def __term(self) -> Expression:
        expr = self.__factor()
        while op := self._consume("+", "-"):
            right = self.__factor()
            expr = ArithmeticExpression(expr, op, right)
        return expr

    def __factor(self) -> Expression:
        expr = self.__exponent()
        while op := self._consume("*", "/", "%"):
            right = self.__exponent()
            expr = ArithmeticExpression(expr, op, right)
        return expr

    def __exponent(self) -> Expression:
        expr = self.__unary()
        while op := self._consume("^"):
            right = self.__unary()
            expr = ArithmeticExpression(expr, op, right)
        return expr

    def __unary(self) -> Expression:
        if op := self._consume("!", Keyword.NOT, "+", "-"):
            return UnaryExpression(op, self.__property())
        else:
            return self.__property()

    def __property(self) -> Expression:
        expr = self.__primary()
        while op := self._consume(".", "["):
            if op == ".":
                swizzle = self._match(IDENTIFIER)
                expr = SwizzleExpression(expr, swizzle)
            else:
                indexer = self.__expression()
                self._match("]")
                expr = IndexingExpression(expr, indexer)
        return expr

    def __primary(self) -> Expression:
        # literal
        if literal := self._consume(Keyword.TRUE, Keyword.FALSE, INT_LITERAL, FLOAT_LITERAL, STRING_LITERAL, FILENAME_LITERAL):
            return LiteralExpression(literal)
        # grouping
        if self._consume("("):
            expr = self.__expression()
            self._match(")")
            return GroupingExpression(expr)
        # stdlib call / function call / identifier
        if identifier := self._consume(IDENTIFIER):
            if self._peek() == "(":
                # stdlib call
                if identifier.lexeme in StandardLibrary:
                    return self.__standard_library_call(identifier)
                # function call
                else:
                    return self.__function_call(identifier)
            # identifier
            else:
                return IdentifierExpression(identifier)
        token = self._peek()
        # constructor call
        if token in DATA_TYPES:
            return self.__constructor_call()
        # if
        if token == Keyword.IF:
            return self.__if_expression()
        # switch
        if token == Keyword.SWITCH:
            return self.__switch_expression()
        # node constructor
        if token == "{":
            return self.__node_constructor()
        raise CompileError(token.line, f"Unexpected token: '{token}'.")

    def __if_expression(self) -> Expression:
        self._match(Keyword.IF)
        self._match("(")
        clause = self.__expression()
        self._match(")")
        self._match("{")
        then = self.__expression()
        self._match("}")
        if self._consume(Keyword.ELSE):
            self._match("{")
            otherwise = self.__expression()
            self._match("}")
        else:
            otherwise = None
        return IfExpression(clause, then, otherwise)

    def __switch_expression(self) -> Expression:
        self._match(Keyword.SWITCH)
        self._match("(")
        which = self.__expression()
        self._match(")")
        self._match("{")
        values = [self.__expression()]
        while self._consume(","):
            values.append(self.__expression())
        self._match("}")
        return SwitchExpression(which, values)

    def __constructor_call(self) -> Expression:
        data_type = self._match(*DATA_TYPES)
        self._match("(")
        if self._consume(")"):
            args = []
        else:
            args = [self.__argument()]
            while self._consume(","):
                args.append(self.__argument())
            self._match(")")
        return ConstructorCall(data_type, args)

    def __standard_library_call(self, identifier: Token) -> Expression:
        self._match("(")
        if self._consume(")"):
            args = []
        else:
            args = [self.__argument()]
            while self._consume(","):
                args.append(self.__argument())
            self._match(")")
        return StandardLibraryCall(identifier, args)

    def __function_call(self, identifier: Token) -> Expression:
        self._match("(")
        if self._consume(")"):
            args = []
        else:
            args = [self.__argument()]
            while self._consume(","):
                args.append(self.__argument())
            self._match(")")
        return FunctionCall(identifier, args)

    def __node_constructor(self) -> NodeConstructor:
        self._match("{")
        category = self._match(STRING_LITERAL)
        self._match(",")
        data_type = self._match(*DATA_TYPES)
        args = []
        if self._consume(":"):
            args.append(self.__argument())
            while self._consume(","):
                args.append(self.__argument())
        self._match("}")
        return NodeConstructor(category, data_type, args)

    def __argument(self) -> Argument:
        if self._peek() == IDENTIFIER and self._peek_next() == "=":
            name = self._match(IDENTIFIER)
            self._match("=")
        else:
            name = None
        return Argument(self.__expression(), name)
