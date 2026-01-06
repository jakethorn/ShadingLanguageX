//
// Created by jaket on 13/11/2025.
//

#include "parse.h"

#include "TokenReader.h"
#include "expressions/ExpressionFactory.h"
#include "expressions/FunctionCall.h"
#include "expressions/Identifier.h"
#include "runtime/Attribute.h"
#include "statements/Statement.h"
#include "expressions/Literal.h"
#include "runtime/Parameter.h"
#include "runtime/Argument.h"
#include "runtime/ParameterList.h"
#include "statements/FunctionDefinition.h"
#include "statements/PrintStatement.h"
#include "statements/ReturnStatement.h"
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
    vector mods = modifiers();

    if (peek() == TokenType::Print)
    {
        return print_statement();
    }

    if (peek() == TokenType::Function)
    {
        return function_definition_modern(std::move(mods));
    }

    if (peek() == TokenType::Return)
    {
        return return_statement();
    }

    if (peek() == TokenType::Identifier and peek(1) == TokenType::Identifier)
    {
        if (peek(2) == '=')
        {
            return variable_definition(std::move(mods));
        }
        else
        {
            return function_definition(std::move(mods));
        }
    }

    throw CompileError{peek(), "Invalid statement"};
}

vector<Token> Parser::modifiers()
{
    return consume_while(TokenType::Const, TokenType::Mutable, TokenType::Global, TokenType::Inline);
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

StmtPtr Parser::variable_definition(vector<Token> modifiers)
{
    Type type = match(TokenType::Identifier);
    Token name = match(TokenType::Identifier);
    match('=');
    ExprPtr expr = expression();
    match(';');

    return std::make_unique<VariableDefinition>(
        runtime_,
        Token::as_strings(modifiers),
        std::move(type),
        std::move(name),
        std::move(expr)
    );
}

StmtPtr Parser::function_definition(vector<Token> modifiers)
{
    Type type = match(TokenType::Identifier);
    current_function_type_ = type;
    Token name = match(TokenType::Identifier);
    vector template_types = peek() == '<' ? list<Type>('<', '>', [this](const size_t){ return match(TokenType::Identifier); }) : vector<Type>{};
    ParameterList params = list<Parameter>('(', ')', [this](const size_t){ return parameter(); });
    vector body = list<StmtPtr>('{', '}', [this](const size_t){ return statement(); });
    return std::make_unique<FunctionDefinition>(
        runtime_, Token::as_strings(modifiers), std::move(type), std::move(name), std::move(template_types), std::move(params), std::move(body)
    );
}

StmtPtr Parser::function_definition_modern(vector<Token> modifiers)
{
    match(TokenType::Function);
    Token name = match(TokenType::Identifier);
    vector template_types = peek() == '<' ? list<Type>('<', '>', [this](const size_t){ return match(TokenType::Identifier); }) : vector<Type>{};
    ParameterList params = list<Parameter>('(', ')', [this](const size_t){ return parameter(); });
    match(TokenType::Arrow);
    Type type = match(TokenType::Identifier);
    current_function_type_ = type; // doesnt work because of nested functions, either return statements needs to be handled specially or they grab the type from runtime.
    vector body = list<StmtPtr>('{', '}', [this](const size_t){ return statement(); });
    return std::make_unique<FunctionDefinition>(
        runtime_, Token::as_strings(modifiers), std::move(type), std::move(name), std::move(template_types), std::move(params), std::move(body)
    );
}

Parameter Parser::parameter()
{
    vector modifiers = consume_while(TokenType::Const, TokenType::Mutable, TokenType::Out);
    Type type = match(TokenType::Identifier);
    Token name = match(TokenType::Identifier);
    ExprPtr expr = consume('=') ? expression() : nullptr;
    return Parameter{Token::as_strings(modifiers), std::move(type), std::move(name), std::move(expr)};
}

StmtPtr Parser::return_statement()
{
    match(TokenType::Return);
    ExprPtr expr = expression();
    match(';');
    return std::make_unique<ReturnStatement>(runtime_, std::move(current_function_type_), std::move(expr));
}

ExprPtr Parser::expression()
{
    return logical();
}

ExprPtr Parser::logical()
{
    ExprPtr expr = equality();
    while (optional<Token> op = consume("and"s, "or"s))
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
    if (optional<Token> op = consume("!"s, "not"s, "+"s, "-"s))
        return ExpressionFactory::unary(std::move(*op), property());
    return property();
}

ExprPtr Parser::property()
{
    return primary();
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

    throw CompileError{peek(), "Invalid expression"};
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
    vector args = list<Argument>('(', ')', [this](const size_t i){ return argument(i);});
    return std::make_unique<FunctionCall>(runtime_, std::move(name), std::move(template_type), std::move(args));
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
