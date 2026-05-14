//
// Created by jaket on 14/05/2026.
//

#include "ConstructorDefinition.h"

#include "parse.h"
#include "scan.h"
#include "mtlx/MtlXSerializer.h"
#include "runtime/Function.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"

ConstructorDefinition::ConstructorDefinition(ModifierList mods, string class_name, ParameterList params, StmtPtr body, Token token)
    : Statement{std::move(token)}, mods_{std::move(mods)}, class_name_{std::move(class_name)}, params_{std::move(params)}, body_{std::move(body)}
{
    mods_.validate(TokenType::Inline);
}

StmtPtr ConstructorDefinition::instantiate_template_types(const TypePtr& template_type) const
{
    ParameterList params = params_.instantiate_template_types(template_type);
    StmtPtr body = body_->instantiate_template_types(template_type);
    return std::make_unique<ConstructorDefinition>(mods_, class_name_, std::move(params), std::move(body), token_);
}

namespace
{
    ParameterList duplicate_parameters(const ParameterList& params)
    {
        vector<Parameter> dup_params;
        dup_params.reserve(params.size());
        for (const Parameter& param : params)
            dup_params.emplace_back(param.attributes(), param.modifiers(), param.type(), param.name(), nullptr, param.index());
        return ParameterList{std::move(dup_params)};
    }

    tuple<StmtPtr, ExprPtr> get_outer_ctor_body(const TypePtr& class_type, const ParameterList& params)
    {
        string arg_list = ""s;
        for (const Parameter& param : params)
            arg_list += param.name() + ", ";

        string code = R"(
            {
                mutable )" + class_type->name() + R"( value;
                value.__ctor__()" + arg_list + R"();
                return value;
            }
        )";

        vector<Token> tokens = sscan(std::move(code));
        Parser parser(std::move(tokens));
        return parser.function_body();
    }
}

void ConstructorDefinition::init()
{
    inner_ctor_ = std::make_shared<Function>(
        ModifierList{TokenType::Inline},
        std::make_shared<Type>(Type::Void),
        "__ctor__"s,
        nullptr, // template_type
        duplicate_parameters(params_),
        std::move(body_),
        nullptr // return_expr
    );

    const TypePtr class_type = scope().get_type(class_name_);
    class_type->add_method(inner_ctor_);
    inner_ctor_->set_class_type(class_type);

    auto [body, return_expr] = get_outer_ctor_body(class_type, params_);
    outer_ctor_ = std::make_shared<Function>(
        mods_,
        class_type,
        "__" + class_name_ + "__",
        nullptr, // template_type
        std::move(params_),
        std::move(body),
        std::move(return_expr)
    );
}

void ConstructorDefinition::execute_impl() const
{
    inner_ctor_->init();
    scope().add_function(inner_ctor_);

    outer_ctor_->init();
    scope().add_function(outer_ctor_);

    if (not outer_ctor_->is_inline())
        serializer().write_node_def_graph(outer_ctor_, AttributeList{});
}
