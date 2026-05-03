//
// Created by jaket on 13/11/2025.
//

#include "parse.h"

#include "TokenReader.h"
#include "expressions/DotExpression.h"
#include "expressions/UnnamedConstructor.h"
#include "expressions/ExpressionFactory.h"
#include "expressions/FunctionCall.h"
#include "expressions/FunctionCall.h"
#include "expressions/Identifier.h"
#include "expressions/IndexingExpression.h"
#include "runtime/Attribute.h"
#include "expressions/Literal.h"
#include "expressions/VariableDefinitionExpression.h"
#include "runtime/Parameter.h"
#include "runtime/Argument.h"
#include "runtime/ParameterList.h"
#include "runtime/Field.h"
#include "statements/BlockStatement.h"
#include "statements/ExpressionStatement.h"
#include "statements/ForEachLoop.h"
#include "statements/ForRangeLoop.h"
#include "statements/FunctionDefinition.h"
#include "statements/FunctionDefinition.h"
#include "statements/IfStatement.h"
#include "statements/MultiVariableDefinition.h"
#include "statements/PrintStatement.h"
#include "statements/VariableDefinition.h"
#include "statements/UsingDeclaration.h"
#include "statements/VariableAssignment.h"

vector<StmtPtr> parse(vector<Token> tokens)
{
    return Parser{std::move(tokens)}.parse();
}

Parser::Parser(vector<Token> tokens_) : TokenReader{std::move(tokens_)}
{

}

vector<StmtPtr> Parser::parse()
{
    Token debug_info;
    try
    {
        vector<StmtPtr> statements;
        while (not empty())
        {
            debug_info = peek();
            statements.push_back(statement());
        }

        return statements;
    }
    catch (const CompileError& e)
    {
        throw CompileError{debug_info, e};
    }
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

    if (peek() == TokenType::For)
    {
        return for_loop();
    }

    if (peek() == TokenType::If)
    {
        return if_statement();
    }

    ModifierList mods = modifiers();

    if (peek() == TokenType::Function)
    {
        return function_definition_modern(std::move(mods));
    }

    if (peek() == '{' or (peek() == TokenType::Identifier and peek(1) == TokenType::Identifier))
    {
        TypePtr type_ = type();

        if (peek(1) == '=' or peek(1) == ';')
        {
            return variable_definition(std::move(mods), std::move(type_));
        }
        if (peek(1) == ',')
        {
            return multi_variable_definition(std::move(mods), std::move(type_));
        }
        if (peek(1) == '<' or peek(1) == '(')
        {
            return function_definition(std::move(mods), std::move(type_));
        }
    }

    if (peek() == TokenType::Identifier)
    {
        mods.validate();

        if (peek(1) == '=' or peek(1) == '.' or peek(1) == '[')
        {
            return variable_assignment();
        }

        if (peek(1) == '<' or peek(1) == '(')
        {
            return expression_statement();
        }
    }

    throw CompileError{peek(), "Invalid statement"s};
}

StmtPtr Parser::print_statement()
{
    Token token = match(TokenType::Print);

    vector<ExprPtr> exprs;
    do
    {
        exprs.push_back(expression());
    }
    while (consume(',') and peek() != ';');
    match(';');

    return std::make_unique<PrintStatement>(std::move(token), std::move(exprs));
}

StmtPtr Parser::variable_definition(ModifierList mods, TypePtr type)
{
    Token name = match(TokenType::Identifier);
    ExprPtr expr = consume('=') ? expression() : nullptr;
    match(';');

    return std::make_unique<VariableDefinition>(
        std::move(mods),
        std::move(type),
        std::move(name),
        std::move(expr)
    );
}

StmtPtr Parser::multi_variable_definition(ModifierList mods, TypePtr type_)
{
    vector<Field> fields;
    Token name = match(TokenType::Identifier);
    fields.emplace_back(std::move(mods), std::move(type_), std::move(name), nullptr);
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
            type_ = type();
            name = match(TokenType::Identifier);
            fields.emplace_back(std::move(mods), std::move(type_), std::move(name), nullptr);
        }
    }
    Token token = match('=');
    ExprPtr expr = expression();
    match(';');

    return std::make_unique<MultiVariableDefinition>(
        std::move(token),
        std::make_unique<Type>(std::move(fields)),
        std::move(expr)
    );
}

StmtPtr Parser::variable_assignment()
{
    ExprPtr lhs = expression();
    Token token = match('=');
    ExprPtr rhs;
    if (peek() == TokenType::If)
        rhs = if_expression(lhs);
    else
        rhs = expression();
    match(';');
    return std::make_unique<VariableAssignment>(std::move(token), std::move(lhs), std::move(rhs));
}

StmtPtr Parser::function_definition(ModifierList mods, TypePtr type)
{
    Token name = match(TokenType::Identifier);
    vector<TypePtr> template_types = template_list();
    ParameterList params = list<Parameter>('(', ')', [this](const size_t i){ return parameter(i); });
    auto [body, return_expr] = function_body();
    return std::make_unique<FunctionDefinition>(
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
    vector<TypePtr> template_types = template_list();
    ParameterList params = list<Parameter>('(', ')', [this](const size_t i){ return parameter(i); });
    match(TokenType::Arrow);
    TypePtr type_ = type();
    auto [body, return_expr] = function_body();
    return std::make_unique<FunctionDefinition>(
        std::move(mods),
        std::move(type_),
        std::move(name),
        std::move(template_types),
        std::move(params),
        std::move(body),
        std::move(return_expr)
    );
}

StmtPtr Parser::using_declaration()
{
    Token token = match(TokenType::Using);
    Token name = match(TokenType::Identifier);
    match('=');
    TypePtr type_ = type();
    match(';');
    return std::make_unique<UsingDeclaration>(std::move(token), std::move(name), std::move(type_));
}

StmtPtr Parser::for_loop()
{
    Token token = match(TokenType::For);
    match('(');
    ModifierList mods = modifiers();
    TypePtr type_ = type();
    Token name = match(TokenType::Identifier);
    match('=');
    ExprPtr expr1 = expression();
    if (consume(':'))
    {
        ExprPtr expr2 = expression();
        ExprPtr expr3 = nullptr;
        if (consume(':'))
        {
            expr3 = expression();
        }
        else
        {
            expr3 = std::move(expr2);
            expr2 = nullptr;
        }

        match(')');
        StmtPtr body = block_statement();
        return std::make_unique<ForRangeLoop>(std::move(token), std::move(mods), std::move(type_), std::move(name), std::move(expr1), std::move(expr2), std::move(expr3), std::move(body));
    }
    else
    {
        match(')');
        StmtPtr body = block_statement();
        return std::make_unique<ForEachLoop>(std::move(token), std::move(mods), std::move(type_), std::move(name), std::move(expr1), std::move(body));
    }
}

StmtPtr Parser::expression_statement()
{
    ExprPtr expr = function_call();
    match(';');
    return std::make_unique<ExpressionStatement>(std::move(expr));
}

StmtPtr Parser::block_statement()
{
    vector<StmtPtr> body;

    Token token = match('{');
    while (not consume('}'))
        body.push_back(statement());

    return std::make_unique<BlockStatement>(std::move(token), std::move(body));
}

StmtPtr Parser::if_statement()
{
    Token token = match(TokenType::If);
    match('(');
    ExprPtr cond_expr = expression();
    match(')');
    StmtPtr then_block = block_statement();

    StmtPtr else_block = nullptr;
    if (consume(TokenType::Else))
    {
        if (peek() == TokenType::If)
        {
            else_block = if_statement();
        }
        else
        {
            else_block = block_statement();
        }
    }

    return std::make_unique<IfStatement>(std::move(token), std::move(cond_expr), std::move(then_block), std::move(else_block));
}

ModifierList Parser::modifiers()
{
    const vector<Token> mod_tokens = consume_while(
        TokenType::Const, TokenType::Mutable, TokenType::Global, TokenType::Inline, TokenType::Default, TokenType::Ref, TokenType::Out
    );

    vector<TokenType> mods;
    mods.reserve(mod_tokens.size());
    for (const Token& mod : mod_tokens)
        mods.push_back(mod.type());

    return ModifierList{std::move(mods)};
}

TypePtr Parser::type()
{
    if (const optional<Token> type = consume(TokenType::Identifier))
        return std::make_shared<Type>(Token::to_string(type));

    vector<Field> fields = list<Field>('{', '}', [this](const size_t){ return field(); });
    return std::make_shared<Type>(std::move(fields));
}

Field Parser::field()
{
    ModifierList mods = modifiers();
    TypePtr type_ = type();
    const optional<Token> name = consume(TokenType::Identifier);
    return Field{std::move(mods), std::move(type_), Token::to_string(name), nullptr};
}

Parameter Parser::parameter(const size_t index)
{
    ModifierList mods = modifiers();
    TypePtr type_ = type();
    Token name = match(TokenType::Identifier);
    ExprPtr expr = consume('=') ? expression() : nullptr;
    return Parameter{std::move(mods), std::move(type_), std::move(name), std::move(expr), index};
}

vector<TypePtr> Parser::template_list()
{
    if (peek() != '<')
        return {};
    return list<TypePtr>('<', '>', [this](const size_t){ return type(); });
}

tuple<StmtPtr, ExprPtr> Parser::function_body()
{
    vector<StmtPtr> body;
    ExprPtr return_expr;
    
    Token token = match('{');
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

    return {std::make_unique<BlockStatement>(std::move(token), std::move(body)), std::move(return_expr)};
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
            expr = std::make_unique<IndexingExpression>(std::move(expr), std::move(index));
            match(']');
        }
        else
        {
            Token name = match(TokenType::Identifier);
            expr = std::make_unique<DotExpression>(std::move(expr), std::move(name));
        }
    }

    return expr;
}

ExprPtr Parser::primary()
{
    if (optional<Token> literal = consume(TokenType::Bool, TokenType::Int, TokenType::Float, TokenType::String))
    {
        return std::make_unique<Literal>(std::move(*literal));
    }

    if (consume('('))
    {
        ExprPtr expr = expression();
        match(')');
        return expr;
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
        return std::make_unique<Identifier>(std::move(name));
    }

    if (peek() == '{')
    {
        return unnamed_constructor();
    }

    if (peek() == TokenType::If)
    {
        return if_expression(nullptr);
    }

    throw CompileError{peek(), "Invalid expression"s};
}

ExprPtr Parser::if_expression(ExprPtr else_expr)
{
    match(TokenType::If);
    match('(');
    ExprPtr cond_expr = expression();
    match(')');
    match('{');
    ExprPtr then_expr = expression();
    match('}');
    if (consume(TokenType::Else))
    {
        if (peek() == TokenType::If)
        {
            else_expr = if_expression(else_expr);
        }
        else
        {
            match('{');
            else_expr = expression();
            match('}');
        }
    }

    if (else_expr == nullptr)
        throw CompileError{peek(), "Missing else branch in if-expression"s};

    return ExpressionFactory::if_expression(std::move(cond_expr), std::move(then_expr), std::move(else_expr));
}

ExprPtr Parser::function_call()
{
    Token name = match(TokenType::Identifier);
    TypePtr template_type = nullptr;
    if (consume('<'))
    {
        template_type = type();
        match('>');
    }
    vector<Argument> args = list<Argument>('(', ')', [this](const size_t i){ return argument(i); });
    return std::make_unique<FunctionCall>(std::move(name), std::move(template_type), std::move(args));
}

ExprPtr Parser::named_constructor()
{
    Token type = match(TokenType::Identifier);
    vector<Argument> arguments = list<Argument>('{', '}', [this](const size_t i) { return argument(i); });
    return ExpressionFactory::named_constructor(std::move(type), std::move(arguments));
}

ExprPtr Parser::unnamed_constructor()
{
    const Token& token = peek();
    vector<ExprPtr> exprs = list<ExprPtr>('{', '}', [this](const size_t) { return expression(); });
    return std::make_unique<UnnamedConstructor>(token, std::move(exprs));
}

ExprPtr Parser::variable_definition_argument(ModifierList mods)
{
    TypePtr type_ = type();
    Token name = match(TokenType::Identifier);
    return std::make_unique<VariableDefinitionExpression>(std::move(mods), std::move(type_), std::move(name));
}

Argument Parser::argument(const size_t i)
{
    string name;
    if (peek(1) == '=')
    {
        name = match(TokenType::Identifier).lexeme();
        match('=');
    }

    const ModifierList mods = modifiers();

    ExprPtr expr;
    if (is_variable_definition())
    {
        mods.validate(TokenType::Const, TokenType::Mutable, TokenType::Out);
        expr = variable_definition_argument(mods.without(TokenType::Out));
    }
    else
    {
        mods.validate(TokenType::Ref, TokenType::Out);
        expr = expression();
    }

    return Argument{mods.only(TokenType::Ref, TokenType::Out), std::move(name), std::move(expr), i};
}

bool Parser::is_variable_definition() const
{
    // consume modifiers before calling this function

    if (peek(0) == TokenType::Identifier and peek(1) == TokenType::Identifier)
    {
        return true;
    }

    if (peek(0) == '{')
    {
        size_t i = 1;
        size_t count = 1;
        while (count > 0)
        {
            if (peek(i) == '{')
                count++;
            else if (peek(i) == '}')
                count--;
            i++;
        }

        return peek(i) == TokenType::Identifier;
    }

    return false;
}

bool Parser::is_templated_function() const
{
    return  peek(0) == TokenType::Identifier and
            peek(1) == '<' and
            peek(2) == TokenType::Identifier and
            peek(3) == '>' and
            peek(4) == '(';
}
