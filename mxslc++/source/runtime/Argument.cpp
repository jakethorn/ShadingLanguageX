//
// Created by jaket on 21/11/2025.
//

#include "runtime/Argument.h"

#include "runtime/Parameter.h"
#include "expressions/Literal.h"
#include "expressions/VariableDefinitionExpression.h"
#include "errors/CompileError.h"
#include "expressions/interface.h"

namespace mxslc::runtime
{
    Argument::Argument(AttributeList attrs, ModifierList mods, string name, ExprPtr expr, const size_t index)
        : attrs_{std::move(attrs)}, mods_{std::move(mods)}, name_{std::move(name)}, expr_{std::move(expr)}, index_{index}
    {
        mods_.validate(TokenType::Ref, TokenType::Out, TokenType::Comptime);
        if (mods_.contains(TokenType::Ref) and mods_.contains(TokenType::Out))
            throw CompileError{"An argument cannot be both ref and out"};
    }

    Argument::Argument(ModifierList mods, string name, ExprPtr expr, const size_t index)
        : Argument{AttributeList{}, std::move(mods), std::move(name), std::move(expr), index} { }

    Argument::Argument(ModifierList mods, ExprPtr expr, const size_t index)
        : Argument{std::move(mods), string{}, std::move(expr), index} { }

    Argument::Argument(string name, ExprPtr expr, const size_t index)
        : Argument{ModifierList{}, std::move(name), std::move(expr), index} { }

    Argument::Argument(ExprPtr expr, const size_t index)
        : Argument{string{}, std::move(expr), index} { }

    Argument::Argument(VarPtr value, const size_t index)
        : Argument{as_expression(std::move(value)), index} { }

    Argument::Argument(ValuePtr value, const size_t index)
        : Argument{as_expression(std::move(value)), index} { }

    Argument::Argument(Primitive value, const size_t index)
        : Argument{as_expression(std::move(value)), index} { }

    Argument::Argument(Argument&& other) noexcept
        : attrs_{std::move(other.attrs_)},
        mods_{std::move(other.mods_)},
        name_{std::move(other.name_)},
        expr_{std::move(other.expr_)},
        index_{other.index_}
    {

    }

    Argument::~Argument() = default;

    Argument Argument::monomorphize(const TypePtr& template_type) const
    {
        return Argument{name_, expr_->monomorphize(template_type), index_};
    }

    void Argument::init(const TypePtr& type) const
    {
        expr_->init(type);
    }

    void Argument::init(const vector<TypePtr>& types) const
    {
        expr_->init(types);
    }

    bool Argument::try_init(const vector<TypePtr>& types) const
    {
        return expr_->try_init(types);
    }

    void Argument::update() const
    {
        expr_->update();
    }

    void Argument::reset() const
    {
        expr_->reset();
    }

    bool Argument::is_initialized() const
    {
        return expr_->is_initialized();
    }

    bool Argument::is_literal() const
    {
        return cast_expression<Literal>(expr_) != nullptr;
    }

    TypePtr Argument::type() const
    {
        return expr_->type();
    }

    VarPtr Argument::evaluate() const
    {
        return expr_->evaluate();
    }

    void Argument::validate(const Parameter& param) const
    {
        if (mods_.contains(TokenType::Ref) and not param.modifiers().contains(TokenType::Ref))
            throw CompileError{"Ref argument is being passed to a non-ref parameter"};

        if (mods_.contains(TokenType::Out) and not param.modifiers().contains(TokenType::Out))
            throw CompileError{"Out argument is being passed to a non-out parameter"};

        if (cast_expression<VariableDefinitionExpression>(expr_) and not param.modifiers().contains(TokenType::Out))
            throw CompileError{"Variable definition expressions can only be passed to out parameter"};
    }

    const string& Argument::error_message() const
    {
        return expr_->error_message();
    }

    string Argument::to_string() const
    {
        string mods_string = mods_.to_string();
        if (not mods_string.empty())
            mods_string += " ";

        if (has_name())
            return mods_string + name_ + " = " + expr_->to_string();
        else
            return mods_string + expr_->to_string();

    }
}
