from .Argument import Argument
from .CompileError import CompileError
from .Expressions import *
from .Expressions.LiteralExpression import NullExpression
from .Keyword import Keyword
from .Parameter import Parameter
from .Statements import *
from .Token import Token, IdentifierToken
from .TokenReader import TokenReader
from .token_types import IDENTIFIER, FLOAT_LITERAL, INT_LITERAL, STRING_LITERAL, FILENAME_LITERAL


def parse(tokens: list[Token]) -> list[Statement]:
    return Parser(tokens).parse()


class Parser(TokenReader):
    def __init__(self, tokens: list[Token]):
        super().__init__(tokens)

    def parse(self) -> list[Statement]:
        return self.__program()

    def __program(self) -> list[Statement]:
        statements = []
        while self._reading_tokens():
            statements.append(self.__statement())
        return statements

    def __statement(self) -> Statement:
        token = self._peek()
        if token in Keyword.DATA_TYPES() ^ {Keyword.AUTO} and self._peek_next_next() == "=":
            return self.__variable_declaration()
        if token in Keyword.DATA_TYPES() ^ {Keyword.AUTO, Keyword.VOID} and self._peek_next_next() in ["(", "<"]:
            return self.__function_declaration()
        if token == IDENTIFIER:
            if self._peek_next() in ["(", "<"]:
                expr = self.__primary()
                self._match(";")
                return ExpressionStatement(expr)
            else:
                return self.__assignment()
        if token == Keyword.FOR:
            return self.__for_loop()
        raise CompileError(f"Expected return statement, data type keyword, identifier or 'for', but found '{token.lexeme}'.", token)

    def __variable_declaration(self) -> VariableDeclaration:
        data_type = self._match(Keyword.DATA_TYPES(), Keyword.AUTO)
        identifier = self._match(IDENTIFIER)
        self._match("=")
        right = self.__expression()
        self._match(";")
        return VariableDeclaration(data_type, identifier, right)

    def __function_declaration(self) -> FunctionDeclaration:
        return_type = self._match(Keyword.DATA_TYPES(), Keyword.AUTO, Keyword.VOID)
        identifier = self._match(IDENTIFIER)
        template_types = []
        if self._consume("<"):
            template_types.append(self._match(Keyword.DATA_TYPES() - {Keyword.T}))
            while self._consume(","):
                template_types.append(self._match(Keyword.DATA_TYPES() - {Keyword.T}))
            self._match(">")
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
        if return_type in Keyword.DATA_TYPES() ^ {Keyword.AUTO}:
            while self._peek() != Keyword.RETURN:
                statements.append(self.__statement())
            self._match(Keyword.RETURN)
            return_expr = self.__expression()
            self._match(";")
            self._match("}")
        else:
            while self._peek() != "}":
                statements.append(self.__statement())
            self._match("}")
            return_expr = NullExpression()
        return FunctionDeclaration(return_type, identifier, template_types, params, statements, return_expr)

    def __parameter(self) -> Parameter:
        data_type = self._match(Keyword.DATA_TYPES())
        identifier = self._match(IDENTIFIER)
        if self._consume("="):
            default_value = self.__expression()
        else:
            default_value = None
        return Parameter(identifier, data_type, default_value)

    def __assignment(self) -> Statement:
        identifier = self._match(IDENTIFIER)
        property_ = self._match(IDENTIFIER) if self._consume(".") else None
        token = self._peek()
        if token == "=":
            return self.__variable_assignment(identifier, property_)
        if token in ["+=", "-=", "*=", "/=", "%=", "^=", "&=", "|="]:
            return self.__compound_assignment(identifier, property_)
        raise CompileError(f"Unexpected token: '{token.lexeme}'.", token)

    def __variable_assignment(self, identifier: Token, property_: Token) -> VariableAssignment:
        self._match("=")
        right = self.__expression()
        self._match(";")
        return VariableAssignment(identifier, property_, right)

    def __compound_assignment(self, identifier: Token, property_: Token) -> CompoundAssignment:
        operator = self._match("+=", "-=", "*=", "/=", "%=", "^=", "&=", "|=")
        right = self.__expression()
        self._match(";")
        return CompoundAssignment(identifier, property_, operator, right)

    def __for_loop(self) -> ForLoop:
        self._match(Keyword.FOR)
        self._match("(")
        data_type = self._match(Keyword.DATA_TYPES())
        identifier = self._match(IDENTIFIER)
        self._match("=")
        start_value = self._match(FLOAT_LITERAL, IDENTIFIER)
        self._match(":")
        value2 = self._match(FLOAT_LITERAL, IDENTIFIER)
        if self._consume(":"):
            value3 = self._match(FLOAT_LITERAL, IDENTIFIER)
        else:
            value3 = None
        self._match(")")
        self._match("{")
        statements = []
        while self._peek() != "}":
            statements.append(self.__statement())
        self._match("}")
        return ForLoop(data_type, identifier, start_value, value2, value3, statements)

    def __expression(self) -> Expression:
        return self.__logic()

    def __logic(self) -> Expression:
        expr = self.__equality()
        while op := self._consume("&", Keyword.AND, "|", Keyword.OR):
            right = self.__equality()
            expr = BinaryExpression(expr, op, right)
        return expr

    def __equality(self) -> Expression:
        expr = self.__relational()
        while op := self._consume("!=", "=="):
            right = self.__relational()
            expr = BinaryExpression(expr, op, right)
        return expr

    def __relational(self) -> Expression:
        left = self.__term()
        middle = None
        right = None

        relational_operators = [">", ">=", "<", "<="]
        if op1 := self._consume(relational_operators):
            middle = self.__term()
        if op2 := self._consume(relational_operators):
            right = self.__term()

        if middle is None:
            return left
        elif right is None:
            return BinaryExpression(left, op1, middle)
        else:
            return TernaryRelationalExpression(left, op1, middle, op2, right)

    def __term(self) -> Expression:
        expr = self.__factor()
        while op := self._consume("+", "-"):
            right = self.__factor()
            expr = BinaryExpression(expr, op, right)
        return expr

    def __factor(self) -> Expression:
        expr = self.__exponent()
        while op := self._consume("*", "/", "%"):
            right = self.__exponent()
            expr = BinaryExpression(expr, op, right)
        return expr

    def __exponent(self) -> Expression:
        expr = self.__unary()
        while op := self._consume("^"):
            right = self.__unary()
            expr = BinaryExpression(expr, op, right)
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
        if literal := self._consume(Keyword.TRUE, Keyword.FALSE, INT_LITERAL, FLOAT_LITERAL, STRING_LITERAL, FILENAME_LITERAL, Keyword.NULL):
            return LiteralExpression(literal)
        # grouping
        if self._consume("("):
            expr = self.__expression()
            self._match(")")
            return GroupingExpression(expr)
        # function call / identifier
        if identifier := self._consume(IDENTIFIER):
            # function call
            if (self._peek() == "(") or (self._peek() == "<" and self._peek_next() in Keyword.DATA_TYPES() and self._peek_next_next() == ">"):
                return self.__function_call(identifier)
            # identifier
            else:
                return IdentifierExpression(identifier)
        token = self._peek()
        # constructor call
        if token in Keyword.DATA_TYPES():
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
        raise CompileError(f"Unexpected token: '{token}'.", token)

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
        data_type = self._match(Keyword.DATA_TYPES())
        self._match("(")
        if self._consume(")"):
            args = []
        else:
            args = self.__argument_list()
            self._match(")")
        return ConstructorCall(data_type, args)

    def __function_call(self, identifier: Token) -> Expression:
        template_type = None
        if self._consume("<"):
            template_type = self._match(Keyword.DATA_TYPES() - {Keyword.T})
            self._match(">")
        self._match("(")
        if self._consume(")"):
            args = []
        else:
            args = self.__argument_list()
            self._match(")")
        return FunctionCall(identifier, template_type, args)

    def __node_constructor(self) -> NodeConstructor:
        self._match("{")
        category = self._match(STRING_LITERAL)
        self._match(",")
        data_type = self._match(Keyword.DATA_TYPES())
        if self._consume(":"):
            args = self.__argument_list()
        else:
            args = []
        self._match("}")
        return NodeConstructor(category, data_type, args)

    def __argument_list(self) -> list[Argument]:
        args = [self.__argument(0)]
        i = 1
        while self._consume(","):
            args.append(self.__argument(i))
            i += 1
        return args

    def __argument(self, index: int) -> Argument:
        if self._peek() == IDENTIFIER and self._peek_next() == "=":
            name = self._match(IDENTIFIER)
            self._match("=")
        elif self._peek() in Keyword.DATA_TYPES() and self._peek_next() == "=":
            keyword = self._match(Keyword.DATA_TYPES())
            name = IdentifierToken(keyword.lexeme)
            self._match("=")
        else:
            name = None
        return Argument(self.__expression(), index, name)
