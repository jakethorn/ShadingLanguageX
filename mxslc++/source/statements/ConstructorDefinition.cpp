//
// Created by jaket on 14/05/2026.
//

#include "statements/ConstructorDefinition.h"

#include "parse.h"
#include "scan.h"
#include "serialize/Serializer.h"
#include "runtime/Function.h"
#include "runtime/interface.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "runtime/utils/monomorphize.h"
#include "statements/interface.h"

namespace mxslc::statements
{
    ConstructorDefinition::ConstructorDefinition(ModifierList mods, string class_name, ParameterList params, StmtPtr body, Token token)
        : Statement{std::move(token)}, mods_{std::move(mods)}, class_name_{std::move(class_name)}, params_{std::move(params)}, body_{std::move(body)}
    {
        mods_.validate(TokenType::Inline);
    }

    StmtPtr ConstructorDefinition::monomorphize(const TypePtr& template_type) const
    {
        auto&& [params, body] = runtime_utils::monomorphize_all(template_type, params_, body_);
        return create_statement<ConstructorDefinition>(mods_, class_name_, std::move(params), std::move(body), token_);
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

        StmtPtr get_outer_ctor_body(const TypePtr& class_type, const ParameterList& params)
        {
            string arg_list;
            for (const Parameter& param : params)
                arg_list += param.name() + ", ";

            string code = R"(
                {
                    mutable )" + class_type->name() + R"( value;
                    value.__ctor__()" + arg_list + R"();
                    return value;
                }
            )";

            vector<Token> tokens = scan_string(std::move(code));
            Parser parser{std::move(tokens)};
            return parser.function_body();
        }
    }

    void ConstructorDefinition::init()
    {
        inner_ctor_ = create_function(
            TokenType::Inline,
            Type::Void,
            "__ctor__",
            nullptr, // template_type
            duplicate_parameters(params_),
            std::move(body_)
        );

        const TypePtr class_type = scope().get_type(class_name_);
        class_type->add_method(inner_ctor_);
        inner_ctor_->set_class_type(class_type);

        StmtPtr body = get_outer_ctor_body(class_type, params_);
        outer_ctor_ = create_function(
            mods_,
            class_type,
            "__" + class_name_ + "__",
            nullptr, // template_type
            std::move(params_),
            std::move(body)
        );
    }

    void ConstructorDefinition::execute_impl() const
    {
        inner_ctor_->init();
        scope().add_function(inner_ctor_);

        outer_ctor_->init();
        scope().add_function(outer_ctor_);

        if (not outer_ctor_->is_inline())
            serializer().write_node_def_graph(outer_ctor_);
    }

    string ConstructorDefinition::to_string() const
    {
        string mods_string = mods_.to_string();
        if (not mods_string.empty())
            mods_string += " ";

        return mods_string + class_name_ + "(" + params_.to_string() + ")\n" + body_->to_string();
    }
}
