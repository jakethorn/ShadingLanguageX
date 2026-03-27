//
// Created by jaket on 13/11/2025.
//

#include "parse.h"

#include "TokenReader.h"
#include "expressions/DotExpression.h"
#include "expressions/UnnamedConstructor.h"
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
#include "runtime/FieldInfo.h"
#include "statements/FunctionDefinition.h"
#include "statements/MultiVariableDefinition.h"
#include "statements/PrintStatement.h"
#include "statements/VariableDefinition.h"
#include "statements/UsingDeclaration.h"

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

    if (peek() == TokenType::Using)
    {
        return using_declaration();
    }

    ModifierList mods = modifiers();

    if (peek() == TokenType::Function)
    {
        return function_definition_modern(std::move(mods));
    }

    if (peek() == '{' or (peek() == TokenType::Identifier and peek(1) == TokenType::Identifier))
    {
        TypeInfoPtr type = type_info();

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

StmtPtr Parser::variable_definition(ModifierList mods, TypeInfoPtr type)
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

StmtPtr Parser::multi_variable_definition(ModifierList mods, TypeInfoPtr type)
{
    vector<FieldInfo> fields;
    Token name = match(TokenType::Identifier);
    fields.emplace_back(std::move(mods), std::move(type), std::move(name), nullptr);
    while (consume(','))
    {
        if (peek() == TokenType::Identifier and (peek(1) == ',' or peek(1) == '='))
        {
            name = match(TokenType::Identifier);
            fields.emplace_back(fields.back().modifiers(), fields.back().type(), std::move(name), nullptr);
        }
        else
        {
            mods = modifiers();
            type = type_info();
            name = match(TokenType::Identifier);
            fields.emplace_back(std::move(mods), std::move(type), std::move(name), nullptr);
        }
    }
    match('=');
    ExprPtr expr = expression();
    match(';');

    return std::make_unique<MultiVariableDefinition>(
        runtime_,
        std::make_unique<TypeInfo>(std::move(fields)),
        std::move(expr)
    );
}

StmtPtr Parser::function_definition(ModifierList mods, TypeInfoPtr type)
{
    Token name = match(TokenType::Identifier);
    vector<TypeInfoPtr> template_types = template_list();
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

StmtPtr Parser::function_definition_modern(ModifierList mods)
{
    match(TokenType::Function);
    Token name = match(TokenType::Identifier);
    vector<TypeInfoPtr> template_types = template_list();
    ParameterList params = list<Parameter>('(', ')', [this](const size_t i){ return parameter(i); });
    match(TokenType::Arrow);
    TypeInfoPtr type = type_info();
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

StmtPtr Parser::using_declaration()
{
    match(TokenType::Using);
    Token name = match(TokenType::Identifier);
    match('=');
    TypeInfoPtr type = type_info();
    match(';');
    return std::make_unique<UsingDeclaration>(runtime_, std::move(name), std::move(type));
}

ModifierList Parser::modifiers()
{
    const vector<Token> mods = consume_while(
        TokenType::Const, TokenType::Mutable, TokenType::Global, TokenType::Inline, TokenType::Default, TokenType::Out
    );

    return ModifierList{mods};
}

TypeInfoPtr Parser::type_info()
{
    if (optional<Token> type = consume(TokenType::Identifier))
        return std::make_shared<TypeInfo>(*type);

    vector<FieldInfo> fields = list<FieldInfo>('{', '}', [this](const size_t){ return field_info(); });
    return std::make_shared<TypeInfo>(std::move(fields));
}

FieldInfo Parser::field_info()
{
    ModifierList mods = modifiers();
    TypeInfoPtr type = type_info();
    optional<Token> name = consume(TokenType::Identifier);
    return FieldInfo{std::move(mods), std::move(type), std::move(name), nullptr};
}

Parameter Parser::parameter(const size_t index)
{
    ModifierList mods = modifiers();
    TypeInfoPtr type = type_info();
    Token name = match(TokenType::Identifier);
    ExprPtr expr = consume('=') ? expression() : nullptr;
    return Parameter{runtime_, std::move(mods), std::move(type), std::move(name), std::move(expr), index};
}

vector<TypeInfoPtr> Parser::template_list()
{
    if (peek() != '<')
        return {};
    return list<TypeInfoPtr>('<', '>', [this](const size_t){ return type_info(); });
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
    optional<Token> next;
    while ((next = consume('[', '.')))
    {
        if (next == '[')
        {
            ExprPtr index = expression();
            expr = std::make_unique<IndexingExpression>(runtime_, std::move(expr), std::move(index));
            match(']');
        }
        else
        {
            Token name = match(TokenType::Identifier);
            expr = std::make_unique<DotExpression>(runtime_, std::move(expr), std::move(name));
        }
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

        if (peek(1) == '{')
        {
            return named_constructor();
        }

        Token name = match(TokenType::Identifier);
        return std::make_unique<Identifier>(runtime_, std::move(name));
    }

    if (peek() == '{')
    {
        return unnamed_constructor();
    }

    throw CompileError{peek(), "Invalid expression"s};
}

ExprPtr Parser::function_call()
{
    Token name = match(TokenType::Identifier);
    TypeInfoPtr template_type = nullptr;
    if (consume('<'))
    {
        template_type = type_info();
        match('>');
    }
    vector<Argument> args = list<Argument>('(', ')', [this](const size_t i){ return argument(i); });
    return std::make_unique<FunctionCall>(runtime_, std::move(name), std::move(template_type), std::move(args));
}

ExprPtr Parser::named_constructor()
{
    Token type = match(TokenType::Identifier);
    vector<Argument> arguments = list<Argument>('{', '}', [this](const size_t i) { return argument(i); });
    return ExpressionFactory::named_constructor(std::move(type), std::move(arguments));
}

ExprPtr Parser::unnamed_constructor()
{
    vector<ExprPtr> exprs = list<ExprPtr>('{', '}', [this](const size_t) { return expression(); });
    if (exprs.empty())
        throw CompileError{peek(), "Invalid constructor"s};
    return std::make_unique<UnnamedConstructor>(runtime_, std::move(exprs));
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
    return Argument{std::move(name), std::move(expr), i};
}

bool Parser::is_templated_function() const
{
    return  peek(0) == TokenType::Identifier and
            peek(1) == '<' and
            peek(2) == TokenType::Identifier and
            peek(3) == '>' and
            peek(4) == '(';
}
