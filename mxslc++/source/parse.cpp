//
// Created by jaket on 13/11/2025.
//

#include "parse.h"

#include "expressions/CompoundAssignment.h"
#include "expressions/DotOperator.h"
#include "expressions/UnnamedConstructor.h"
#include "expressions/ExpressionFactory.h"
#include "expressions/FunctionCall.h"
#include "expressions/Identifier.h"
#include "expressions/IfExpression.h"
#include "expressions/IncrementOperator.h"
#include "expressions/IndexingOperator.h"
#include "expressions/interface.h"
#include "expressions/Literal.h"
#include "expressions/MethodCall.h"
#include "expressions/NamedConstructor.h"
#include "expressions/RangeExpression.h"
#include "expressions/ThisExpression.h"
#include "expressions/TypeOfOperator.h"
#include "expressions/VariableDefinitionExpression.h"
#include "expressions/NullExpression.h"
#include "runtime/interface.h"
#include "runtime/Parameter.h"
#include "runtime/Argument.h"
#include "runtime/ParameterList.h"
#include "runtime/Field.h"
#include "runtime/Attribute.h"
#include "runtime/Type.h"
#include "statements/BlockStatement.h"
#include "statements/ClassDefinition.h"
#include "statements/ConstructorDefinition.h"
#include "statements/DocumentAttribute.h"
#include "statements/ExpressionStatement.h"
#include "statements/ForEachLoop.h"
#include "statements/FunctionDefinition.h"
#include "statements/IfStatement.h"
#include "statements/interface.h"
#include "statements/MultiVariableDefinition.h"
#include "statements/PrintStatement.h"
#include "statements/ReturnStatement.h"
#include "statements/VariableDefinition.h"
#include "statements/UsingDeclaration.h"
#include "statements/VariableAssignment.h"

namespace mxslc
{
    vector<StmtPtr> parse(vector<Token> tokens)
    {
        return Parser{std::move(tokens)}.parse();
    }

    Parser::Parser(vector<Token> tokens) : TokenReader{std::move(tokens)}
    {
        ignore(TokenType::Newline);
    }

    vector<StmtPtr> Parser::parse()
    {
        Token debug_info;
        try
        {
            vector<StmtPtr> statements;
            while (not empty())
            {
                if (consume(';'))
                    continue;

                debug_info = peek();

                if (consume(TokenType::Break))
                    match(';');

                statements.push_back(statement());
            }

            return statements;
        }
        catch (CompileError& e)
        {
            e.set_debug_info(debug_info);
            throw;
        }
    }

    StmtPtr Parser::statement()
    {
        AttributeList attrs = attributes();
        StmtPtr stmt = bare_statement();
        stmt->set_attributes(std::move(attrs));
        return stmt;
    }

    StmtPtr Parser::bare_statement()
    {
        if (peek() == "@@"s)
        {
            return document_attribute();
        }

        if (peek() == TokenType::Print)
        {
            return print_statement();
        }

        if (peek() == TokenType::Class)
        {
            return class_definition();
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

        if (peek() == TokenType::Return)
        {
            return return_statement();
        }

        ModifierList mods = modifiers();

        if (is_typed_definition())
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
            if (peek(1) == '<' or peek(1) == '(' or peek(1) == TokenType::FatArrow)
            {
                return function_definition(std::move(mods), std::move(type_));
            }
        }

        mods.validate(TokenType::Comptime);

        ExprPtr expr = expression(mods);

        if (peek() == '=')
        {
            return variable_assignment(std::move(expr));
        }

        if (peek() == ';')
        {
            return expression_statement(std::move(expr));
        }

        throw CompileError{peek(), "Invalid statement"};
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

        return create_statement<PrintStatement>(std::move(exprs), std::move(token));
    }

    StmtPtr Parser::variable_definition(ModifierList mods, TypePtr type)
    {
        Token name = match(TokenType::Identifier);
        ExprPtr expr = consume('=') ? expression() : nullptr;
        match(';');

        return create_statement<VariableDefinition>(
            std::move(mods),
            std::move(type),
            string{name.lexeme()},
            std::move(expr),
            std::move(name)
        );
    }

    StmtPtr Parser::multi_variable_definition(ModifierList mods, TypePtr type_)
    {
        vector<Field> fields;
        Token name = match(TokenType::Identifier);
        Token token{name};
        fields.emplace_back(std::move(mods), std::move(type_), name.lexeme(), /*is_multi_var_def*/true);
        while (consume(','))
        {
            if (peek() == TokenType::Identifier and (peek(1) == ',' or peek(1) == '=' or peek(1) == ';'))
            {
                name = match(TokenType::Identifier);
                fields.emplace_back(fields.back().modifiers(), fields.back().type(), name.lexeme(), /*is_multi_var_def*/true);
            }
            else
            {
                mods = modifiers();
                type_ = type();
                name = match(TokenType::Identifier);
                fields.emplace_back(std::move(mods), std::move(type_), name.lexeme(), /*is_multi_var_def*/true);
            }
        }

        ExprPtr expr = consume('=') ? expression() : nullptr;
        match(';');

        return create_statement<MultiVariableDefinition>(
            create_type(std::move(fields)),
            std::move(expr),
            std::move(token)
        );
    }

    StmtPtr Parser::variable_assignment(ExprPtr lhs)
    {
        Token token = match('=');
        ExprPtr rhs;
        if (peek() == TokenType::If)
            rhs = if_expression(lhs);
        else
            rhs = expression();
        match(';');

        return create_statement<VariableAssignment>(std::move(token), std::move(lhs), std::move(rhs));
    }

    StmtPtr Parser::function_definition(ModifierList mods, TypePtr type)
    {
        Token name = match(TokenType::Identifier);
        vector<TypePtr> template_types = template_list();

        optional<ParameterList> params = optional_list<Parameter, ParameterList>('(', ')', [this](const size_t i){ return parameter(i); });
        if (not params)
            match(TokenType::FatArrow);

        StmtPtr body = function_body();

        return create_statement<FunctionDefinition>(
            std::move(mods),
            std::move(type),
            string{name.lexeme()},
            std::move(template_types),
            std::move(params),
            std::move(body),
            std::move(name)
        );
    }

    StmtPtr Parser::class_definition()
    {
        Token token = match(TokenType::Class);
        string name = match(TokenType::Identifier).lexeme();
        vector<TypePtr> template_types = template_list();
        TypePtr parent = consume(':') ? type() : nullptr;

        match('{');
        vector<StmtPtr> body;
        while (not consume('}'))
        {
            if (is_constructor_definition())
                body.push_back(constructor_definition());
            else
                body.push_back(statement());
        }
        consume(';');

        return create_statement<ClassDefinition>(std::move(name), std::move(template_types), std::move(parent), std::move(body), std::move(token));
    }

    StmtPtr Parser::constructor_definition()
    {
        ModifierList mods = modifiers();
        Token class_name = match(TokenType::Identifier);
        ParameterList params = list<Parameter>('(', ')', [this](const size_t i){ return parameter(i); });
        StmtPtr body = block_statement();

        return create_statement<ConstructorDefinition>(
            std::move(mods),
            string{class_name.lexeme()},
            std::move(params),
            std::move(body),
            std::move(class_name)
        );
    }

    StmtPtr Parser::using_declaration()
    {
        Token token = match(TokenType::Using);
        string name = match(TokenType::Identifier).lexeme();
        match('=');
        TypePtr type_ = type();
        match(';');

        return create_statement<UsingDeclaration>(std::move(token), std::move(name), std::move(type_));
    }

    StmtPtr Parser::for_loop()
    {
        Token token = match(TokenType::For);
        match('(');
        ModifierList mods = modifiers();
        TypePtr type_ = type();
        string name = match(TokenType::Identifier).lexeme();
        match(TokenType::From);
        ExprPtr range_expr = expression();
        match(')');
        StmtPtr body = block_statement();

        return create_statement<ForEachLoop>(std::move(token), std::move(mods), std::move(type_), std::move(name), std::move(range_expr), std::move(body));
    }

    StmtPtr Parser::expression_statement(ExprPtr expr)
    {
        match(';');
        return create_statement<ExpressionStatement>(std::move(expr));
    }

    StmtPtr Parser::block_statement()
    {
        Token token = match('{');
        vector<StmtPtr> body;
        while (not consume('}'))
            body.push_back(statement());

        return create_statement<BlockStatement>(std::move(body), std::move(token));
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

        return create_statement<IfStatement>(std::move(token), std::move(cond_expr), std::move(then_block), std::move(else_block));
    }

    StmtPtr Parser::document_attribute()
    {
        Token token = match("@@"s);
        Attribute attr = attribute();
        return create_statement<DocumentAttribute>(std::move(token), std::move(attr));
    }

    Attribute Parser::attribute()
    {
        string child;
        if (peek(1) == '.')
        {
            child = match_identifier_or_keyword().lexeme();
            match('.');
        }
        string name = match_identifier_or_keyword().lexeme();
        string value = match(TokenType::String).literal<string>();
        return Attribute{std::move(child), std::move(name), std::move(value)};
    }

    AttributeList Parser::attributes()
    {
        vector<Attribute> result;
        while (consume('@'))
            result.push_back(attribute());
        return AttributeList{result};
    }

    ModifierList Parser::modifiers()
    {
        consume("[[");
        const vector<Token> mod_tokens = consume_while(
            TokenType::Const,
            TokenType::Mutable,
            TokenType::Global,
            TokenType::Geomprop,
            TokenType::Nodegraph,
            TokenType::Nodedef,
            TokenType::Inline,
            TokenType::Default,
            TokenType::Comptime,
            TokenType::Ref,
            TokenType::Out
        );
        consume("]]");

        return ModifierList{mod_tokens};
    }

    TypePtr Parser::type()
    {
        if (const optional<Token> type = consume(TokenType::Identifier))
            return create_type(type->lexeme());

        vector<Field> fields = list<Field>('{', '}', [this](const size_t){ return field(); });
        return create_type(std::move(fields));
    }

    Field Parser::field()
    {
        ModifierList mods = modifiers();
        TypePtr type_ = type();
        const optional<Token> name = consume(TokenType::Identifier);
        return Field{std::move(mods), std::move(type_), name ? name->lexeme() : ""};
    }

    Parameter Parser::parameter(const size_t index)
    {
        AttributeList attrs = attributes();
        ModifierList mods = modifiers();
        TypePtr type_ = type();
        string name = match(TokenType::Identifier).lexeme();
        ExprPtr expr = consume('=') ? expression() : nullptr;
        return Parameter{std::move(attrs), std::move(mods), std::move(type_), std::move(name), std::move(expr), index};
    }

    vector<TypePtr> Parser::template_list()
    {
        if (peek() == '<')
            return list<TypePtr>('<', '>', [this](const size_t){ return type(); });
        return {};
    }

    StmtPtr Parser::function_body()
    {
        vector<StmtPtr> body;

        Token token = match('{');
        while (not consume('}'))
            body.push_back(statement());

        return create_statement<BlockStatement>(std::move(body), std::move(token));
    }

    StmtPtr Parser::return_statement()
    {
        Token token = match(TokenType::Return);

        ExprPtr return_expr = nullptr;
        if (not consume(';'))
        {
            return_expr = expression();
            match(';');
        }

        return create_statement<ReturnStatement>(std::move(return_expr), std::move(token));
    }

    ExprPtr Parser::expression(const ModifierList& mods)
    {
        const bool is_comptime = mods.contains(TokenType::Comptime) or consume(TokenType::Comptime).has_value();

        ExprPtr expr = logical();
        if (is_comptime)
            expr->set_modifiers(TokenType::Comptime);
        return expr;
    }

    ExprPtr Parser::logical()
    {
        ExprPtr expr = equality();

        const vector<TokenType> ops = in_abs_ ? vector<TokenType>{'&'} : vector<TokenType>{'&', '|'};
        while (optional<Token> op = consume(ops))
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
        ExprPtr left = range();
        ExprPtr middle;
        ExprPtr right;

        optional<Token> op1 = consume('>', ">="s, '<', "<="s);
        if (op1)
            middle = range();

        optional<Token> op2 = consume('>', ">="s, '<', "<="s);
        if (op2)
            right = range();

        if (right)
            return ExpressionFactory::ternary_relational(std::move(left), std::move(*op1), std::move(middle), std::move(*op2), std::move(right));
        if (middle)
            return ExpressionFactory::binary(std::move(left), std::move(*op1), std::move(middle));
        return left;
    }

    ExprPtr Parser::range()
    {
        ExprPtr expr1 = term();

        if (optional<Token> to = consume(TokenType::To))
        {
            ExprPtr expr2 = term();
            return create_expression<RangeExpression>(std::move(expr1), std::move(expr2), std::move(*to));
        }

        if (optional<Token> colon = consume(':'))
        {
            ExprPtr expr2 = term();
            if (consume(':'))
            {
                ExprPtr expr3 = term();
                return create_expression<RangeExpression>(std::move(expr1), std::move(expr2), std::move(expr3), std::move(*colon));
            }
            else
            {
                return create_expression<RangeExpression>(std::move(expr1), std::move(expr2), std::move(*colon));
            }
        }

        return expr1;
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
            return ExpressionFactory::unary(std::move(*op), compound());
        return compound();
    }

    ExprPtr Parser::compound()
    {
        ExprPtr lhs = increment();

        if (optional<Token> op = consume("+="s, "-="s, "*="s, "/="s, "%="s, "^="s, "&="s, "|="s))
        {
            ExprPtr rhs = expression();
            return create_expression<CompoundAssignment>(std::move(lhs), std::move(*op), std::move(rhs));
        }

        return lhs;
    }

    ExprPtr Parser::increment()
    {
        optional<Token> op = consume("++"s, "--"s);
        bool prefix = op.has_value();

        ExprPtr expr = property();

        if (not op)
            op = consume("++"s, "--"s);

        if (op)
            return create_expression<IncrementOperator>(std::move(expr), std::move(op.value()), prefix);
        else
            return expr;
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
                expr = create_expression<IndexingOperator>(std::move(expr), std::move(index));
                match(']');
            }
            else
            {
                if (is_function_call())
                {
                    expr = method_call(std::move(expr));
                }
                else
                {
                    Token name = match(TokenType::Identifier);
                    expr = create_expression<DotOperator>(std::move(expr), std::move(name));
                }
            }
        }

        return expr;
    }

    ExprPtr Parser::primary()
    {
        if (optional<Token> literal = consume(TokenType::Bool, TokenType::Int, TokenType::Float, TokenType::String))
        {
            return create_expression<Literal>(std::move(*literal));
        }

        if (optional<Token> token = consume(TokenType::Null))
        {
            return create_expression<NullExpression>(std::move(*token));
        }

        if (optional<Token> token = consume(TokenType::This))
        {
            return create_expression<ThisExpression>(std::move(*token));
        }

        if (consume('('))
        {
            ExprPtr expr = expression();
            match(')');
            return expr;
        }

        if (optional<Token> token = consume('|'))
        {
            in_abs_ = true;
            ExprPtr expr = expression();
            match('|');
            in_abs_ = false;
            return ExpressionFactory::absolute(std::move(expr), std::move(*token));
        }

        if (peek() == TokenType::Identifier)
        {
            if (is_function_call())
            {
                return function_call();
            }

            if (peek(1) == '{')
            {
                return named_constructor();
            }

            Token name = match(TokenType::Identifier);
            return create_expression<Identifier>(std::move(name));
        }

        if (peek() == '{')
        {
            return unnamed_constructor();
        }

        if (peek() == TokenType::If)
        {
            return if_expression();
        }

        if (peek() == TokenType::Typeof)
        {
            return typeof_operator();
        }

        throw CompileError{peek(), "Invalid expression"};
    }

    ExprPtr Parser::if_expression(ExprPtr else_expr)
    {
        Token token = match(TokenType::If);
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
            throw CompileError{peek(), "Missing else branch in if-expression"};

        return create_expression<IfExpression>(std::move(cond_expr), std::move(then_expr), std::move(else_expr), std::move(token));
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
        optional<ArgumentList> args = optional_list<Argument, ArgumentList>('(', ')', [this](const size_t i){ return argument(i); });
        return create_expression<FunctionCall>(string{name.lexeme()}, std::move(template_type), std::move(args), std::move(name));
    }

    ExprPtr Parser::method_call(ExprPtr instance)
    {
        Token name = match(TokenType::Identifier);
        TypePtr template_type = nullptr;
        if (consume('<'))
        {
            template_type = type();
            match('>');
        }
        optional<ArgumentList> args = optional_list<Argument, ArgumentList>('(', ')', [this](const size_t i){ return argument(i); });
        return create_expression<MethodCall>(std::move(instance), string{name.lexeme()}, std::move(template_type), std::move(args), std::move(name));
    }

    ExprPtr Parser::named_constructor()
    {
        Token type_name = match(TokenType::Identifier);
        vector<Argument> arguments = list<Argument>('{', '}', [this](const size_t i) { return argument(i); });
        return create_expression<NamedConstructor>(string{type_name.lexeme()}, std::move(arguments), std::move(type_name));
    }

    ExprPtr Parser::unnamed_constructor()
    {
        const Token token = peek();
        vector<ExprPtr> exprs = list<ExprPtr>('{', '}', [this](const size_t) { return expression(); });
        return create_expression<UnnamedConstructor>(std::move(exprs), token);
    }

    ExprPtr Parser::variable_definition_argument(ModifierList mods)
    {
        TypePtr type_ = type();
        Token name = match(TokenType::Identifier);
        return create_expression<VariableDefinitionExpression>(std::move(mods), std::move(type_), std::move(name));
    }

    ExprPtr Parser::typeof_operator()
    {
        Token token = match(TokenType::Typeof);
        match('(');
        ExprPtr expr = expression();
        match(')');
        return create_expression<TypeOfOperator>(std::move(expr), std::move(token));
    }

    Argument Parser::argument(const size_t i)
    {
        AttributeList attrs = attributes();

        string name;
        if (peek(1) == '=')
        {
            // Must check for default and geomprop here because the image/geomprop nodes have inputs with the same name
            name = match(TokenType::Identifier, TokenType::Default, TokenType::Geomprop).lexeme();
            match('=');
        }

        const ModifierList mods = modifiers();

        ExprPtr expr;
        if (is_typed_definition())
        {
            mods.validate(TokenType::Const, TokenType::Mutable, TokenType::Out);
            expr = variable_definition_argument(mods.without(TokenType::Out));
        }
        else
        {
            mods.validate(TokenType::Ref, TokenType::Out);
            expr = expression();
        }

        return Argument{std::move(attrs), mods.only(TokenType::Ref, TokenType::Out), std::move(name), std::move(expr), i};
    }

    bool Parser::is_typed_definition() const
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

    bool Parser::is_function_call() const
    {
        const bool is_func_call =
            peek(0) == TokenType::Identifier and
            peek(1) == '(';

        const bool is_templated_func_call =
            peek(0) == TokenType::Identifier and
            peek(1) == '<' and
            peek(2) == TokenType::Identifier and
            peek(3) == '>' and
            peek(4) == '(';

        const bool is_templated_paramless_func_call =
            peek(0) == TokenType::Identifier and
            peek(1) == '<' and
            peek(2) == TokenType::Identifier and
            peek(3) == '>';

        return is_func_call or is_templated_paramless_func_call or is_templated_func_call;
    }

    bool Parser::is_constructor_definition() const
    {
        return (peek(0) == TokenType::Identifier and peek(1) == '(') or
               (peek(0) == TokenType::Inline and peek(1) == TokenType::Identifier and peek(2) == '(');
    }
}
