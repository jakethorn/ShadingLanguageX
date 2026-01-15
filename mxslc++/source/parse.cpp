//
// Created by jaket on 13/11/2025.
//

#include "parse.h"

#include "TokenReader.h"
#include "expressions/Constructor.h"
#include "expressions/ExpressionFactory.h"
#include "expressions/FunctionCall.h"
#include "expressions/Identifier.h"
#include "expressions/IndexingExpression.h"
#include "runtime/Attribute.h"
#include "statements/Statement.h"
#include "expressions/Literal.h"
#include "runtime/Parameter.h"
#include "runtime/Argument.h"
#include "runtime/ParameterList.h"
#include "runtime/Variable.h"
#include "statements/FunctionDefinition.h"
#include "statements/MultiVariableDefinition.h"
#include "statements/NoStatement.h"
#include "statements/PrintStatement.h"
#include "statements/VariableDefinition.h"

vector<StmtPtr> parse(const Runtime& runtime, vector<Token> tokens)
{
    return Parser{runtime, std::move(tokens)}.parse();
}

Parser::Parser(const Runtime& runtime, vector<Token> tokens_) : TokenReader{std::move(tokens_)}, runtime_{runtime}
{
    ExpressionFactory::init(runtime_);
}

vector<StmtPtr> Parser::parse()
{
    vector<StmtPtr> statements;
    while (not empty())
    {
        statements.push_back(statement());
    }

    return statements;
}

Attribute Parser::attribute()
{
    match('@');
    optional<string> elem = std::nullopt;
    if (peek(1) == '.')
    {
        elem = match(TokenType::Identifier).lexeme();
        match('.');
    }
    string name = match(TokenType::Identifier).lexeme();
    string val = std::get<string>(match(TokenType::String).literal());
    return Attribute{std::move(elem), std::move(name), std::move(val)};
}

StmtPtr Parser::statement()
{
    if (consume(TokenType::Break))
    {
        match(';');
    }

    if (peek() == TokenType::Print)
    {
        return print_statement();
    }

    vector<string> mods = modifiers();

    if (peek() == TokenType::Function)
    {
        return function_definition_modern(std::move(mods));
    }

    if (peek() == '{' or (peek() == TokenType::Identifier and peek(1) == TokenType::Identifier))
    {
        Type type = complex_type();

        if (peek(1) == '=')
        {
            return variable_definition(std::move(mods), std::move(type));
        }
        if (peek(1) == ',')
        {
            return multi_variable_definition(std::move(mods), std::move(type));
        }
        if (peek(1) == '<' or peek(1) == '(')
        {
            return function_definition(std::move(mods), std::move(type));
        }
    }

    throw CompileError{peek(), "Invalid statement"s};
}

StmtPtr Parser::print_statement()
{
    match(TokenType::Print);

    vector<ExprPtr> exprs;
    do
    {
        exprs.push_back(expression());
    }
    while (consume(',') and peek() != ';');
    match(';');

    return std::make_unique<PrintStatement>(runtime_, std::move(exprs));
}

StmtPtr Parser::variable_definition(vector<string> mods, Type type)
{
    Token name = match(TokenType::Identifier);
    match('=');
    ExprPtr expr = expression();
    match(';');

    return std::make_unique<VariableDefinition>(
        runtime_,
        std::move(mods),
        std::move(type),
        std::move(name),
        std::move(expr)
    );
}

StmtPtr Parser::multi_variable_definition(vector<string> mods, Type type)
{
    vector<VariableDeclaration> decls;
    Token name = match(TokenType::Identifier);
    decls.emplace_back(std::move(mods), std::move(type), std::move(name));
    while (consume(','))
    {
        if (peek(1) == TokenType::Identifier)
        {
            mods = modifiers();
            type = complex_type();
            name = match(TokenType::Identifier);
            decls.emplace_back(std::move(mods), std::move(type), std::move(name));
        }
        else
        {
            name = match(TokenType::Identifier);
            decls.emplace_back(decls.back().mods, decls.back().type, std::move(name));
        }
    }
    match('=');
    ExprPtr expr = expression();
    match(';');

    return std::make_unique<MultiVariableDefinition>(
        runtime_,
        std::move(decls),
        std::move(expr)
    );
}

StmtPtr Parser::function_definition(vector<string> mods, Type type)
{
    Token name = match(TokenType::Identifier);
    vector<Type> template_types = peek() == '<' ? list<Type>('<', '>', [this](const size_t){ return match(TokenType::Identifier); }) : vector<Type>{};
    ParameterList params = list<Parameter>('(', ')', [this](const size_t i){ return parameter(i); });
    auto [body, return_expr] = function_body();
    return std::make_unique<FunctionDefinition>(
        runtime_,
        std::move(mods),
        std::move(type),
        std::move(name),
        std::move(template_types),
        std::move(params),
        std::move(body),
        std::move(return_expr)
    );
}

StmtPtr Parser::function_definition_modern(vector<string> mods)
{
    match(TokenType::Function);
    Token name = match(TokenType::Identifier);
    vector<Type> template_types = peek() == '<' ? list<Type>('<', '>', [this](const size_t){ return match(TokenType::Identifier); }) : vector<Type>{};
    ParameterList params = list<Parameter>('(', ')', [this](const size_t i){ return parameter(i); });
    match(TokenType::Arrow);
    Type type = complex_type();
    auto [body, return_expr] = function_body();
    return std::make_unique<FunctionDefinition>(
        runtime_,
        std::move(mods),
        std::move(type),
        std::move(name),
        std::move(template_types),
        std::move(params),
        std::move(body),
        std::move(return_expr)
    );
}

vector<string> Parser::modifiers()
{
    vector<Token> mods = consume_while(TokenType::Const, TokenType::Mutable, TokenType::Global, TokenType::Inline, TokenType::Out);
    return Token::as_strings(mods);
}

Type Parser::complex_type()
{
    if (optional<Token> type = consume(TokenType::Identifier))
        return *type;

    vector<Type> subtypes = list<Type>('{', '}', [this](const size_t){ return complex_type(); });
    return Type{std::move(subtypes)};
}

Parameter Parser::parameter(const size_t index)
{
    vector<string> mods = modifiers();
    Type type = complex_type();
    Token name = match(TokenType::Identifier);
    ExprPtr expr = consume('=') ? expression() : nullptr;
    return Parameter{std::move(mods), std::move(type), std::move(name), std::move(expr), index};
}

tuple<vector<StmtPtr>, ExprPtr> Parser::function_body()
{
    vector<StmtPtr> body;
    ExprPtr return_expr;
    
    match('{');
    while (peek() != '}' and peek() != "return"s)
        body.push_back(statement());
    
    if (consume("return"s))
    {
        return_expr = expression();
        match(';');
    }

    // ignore statements after return
    while (not consume('}'))
        statement();

    return {std::move(body), std::move(return_expr)};
}

ExprPtr Parser::expression()
{
    return logical();
}

ExprPtr Parser::logical()
{
    ExprPtr expr = equality();
    while (optional<Token> op = consume('&', '|'))
    {
        ExprPtr right = equality();
        expr = ExpressionFactory::binary(std::move(expr), std::move(*op), std::move(right));
    }

    return expr;
}

ExprPtr Parser::equality()
{
    ExprPtr expr = relational();
    while (optional<Token> op = consume("=="s, "!="s))
    {
        ExprPtr right = relational();
        expr = ExpressionFactory::binary(std::move(expr), std::move(*op), std::move(right));
    }

    return expr;
}

ExprPtr Parser::relational()
{
    ExprPtr left = term();
    optional<Token> op1;
    ExprPtr middle;
    optional<Token> op2;
    ExprPtr right;

    if ((op1 = consume('>', ">="s, '<', "<="s)))
        middle = term();
    if ((op2 = consume('>', ">="s, '<', "<="s)))
        right = term();

    if (right)
        return ExpressionFactory::ternary_relational(std::move(left), std::move(*op1), std::move(middle), std::move(*op2), std::move(right));
    if (middle)
        return ExpressionFactory::binary(std::move(left), std::move(*op1), std::move(middle));
    return left;
}

ExprPtr Parser::term()
{
    ExprPtr expr = factor();
    while (optional<Token> op = consume('+', '-'))
    {
        ExprPtr right = factor();
        expr = ExpressionFactory::binary(std::move(expr), std::move(*op), std::move(right));
    }

    return expr;
}

ExprPtr Parser::factor()
{
    ExprPtr expr = exponent();
    while (optional<Token> op = consume('*', '/', '%'))
    {
        ExprPtr right = exponent();
        expr = ExpressionFactory::binary(std::move(expr), std::move(*op), std::move(right));
    }

    return expr;
}

ExprPtr Parser::exponent()
{
    ExprPtr expr = unary();
    while (optional<Token> op = consume('^'))
    {
        ExprPtr right = unary();
        expr = ExpressionFactory::binary(std::move(expr), std::move(*op), std::move(right));
    }

    return expr;
}

ExprPtr Parser::unary()
{
    if (optional<Token> op = consume('!', '+', '-'))
        return ExpressionFactory::unary(std::move(*op), property());
    return property();
}

ExprPtr Parser::property()
{
    ExprPtr expr = primary();
    while (consume('['))
    {
        ExprPtr index = expression();
        expr = std::make_unique<IndexingExpression>(runtime_, std::move(expr), std::move(index));
        match(']');
    }

    return expr;
}

ExprPtr Parser::primary()
{
    if (optional<Token> literal = consume(TokenType::Bool, TokenType::Int, TokenType::Float, TokenType::String))
    {
        return std::make_unique<Literal>(runtime_, std::move(*literal));
    }

    if (peek() == TokenType::Identifier)
    {
        if (peek(1) == '(' or is_templated_function())
        {
            return function_call();
        }
        else
        {
            Token name = match(TokenType::Identifier);
            return std::make_unique<Identifier>(runtime_, std::move(name));
        }
    }

    if (peek() == '{')
    {
        return constructor();
    }

    throw CompileError{peek(), "Invalid expression"s};
}

ExprPtr Parser::function_call()
{
    Token name = match(TokenType::Identifier);
    optional<Token> template_type = std::nullopt;
    if (consume('<'))
    {
        template_type = match(TokenType::Identifier);
        match('>');
    }
    vector<Argument> args = list<Argument>('(', ')', [this](const size_t i){ return argument(i); });
    return std::make_unique<FunctionCall>(runtime_, std::move(name), std::move(template_type), std::move(args));
}

ExprPtr Parser::constructor()
{
    vector<ExprPtr> exprs = list<ExprPtr>('{', '}', [this](const size_t) { return expression(); });
    if (exprs.empty())
        throw CompileError{peek(), "Invalid constructor"s};
    return std::make_unique<Constructor>(runtime_, std::move(exprs));
}

Argument Parser::argument(const size_t i)
{
    optional<string> name = std::nullopt;
    if (peek(1) == '=')
    {
        name = match(TokenType::Identifier).lexeme();
        match('=');
    }

    ExprPtr expr = expression();
    return Argument(std::move(name), std::move(expr), i);
}

bool Parser::is_templated_function() const
{
    return  peek(0) == TokenType::Identifier and
            peek(1) == '<' and
            peek(2) == TokenType::Identifier and
            peek(3) == '>' and
            peek(4) == '(';
}
