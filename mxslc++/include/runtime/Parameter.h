//
// Created by jaket on 07/11/2025.
//

#ifndef FENNEC_PARAMETER_H
#define FENNEC_PARAMETER_H

#include "common.h"
#include "runtime/AttributeList.h"
#include "runtime/ModifierList.h"
#include "runtime/utils/monomorphize.h"
#include "runtime/utils/RuntimeAware.h"

namespace mxslc::runtime
{
    using runtime_utils::RuntimeAware;
    using runtime_utils::Monomorphizable;

    class Parameter : protected RuntimeAware, public Monomorphizable<Parameter>
    {
    public:
        Parameter(AttributeList attrs, ModifierList mods, TypePtr type, string name, ExprPtr expr, size_t index);

        Parameter(Parameter&&) noexcept;

        ~Parameter() override;

        const AttributeList& attributes() const { return attrs_; }
        const ModifierList& modifiers() const { return mods_; }
        bool is_const() const { return mods_.contains(TokenType::Const); }
        bool is_mutable() const { return mods_.contains(TokenType::Mutable); }
        bool is_in() const { return not mods_.contains(TokenType::Out); }
        bool is_out() const { return mods_.contains(TokenType::Ref) or mods_.contains(TokenType::Out); }
        const TypePtr& type() const { return type_; }
        const string& name() const { return name_; }
        size_t index() const { return index_; }

        Parameter monomorphize(const TypePtr& template_type) const override;
        void init();

        bool has_default_value() const { return expr_ != nullptr; }
        VarPtr evaluate() const;

        string str() const;

    private:
        AttributeList attrs_;
        ModifierList mods_;
        TypePtr type_;
        string name_;
        ExprPtr expr_;
        size_t index_;
    };
}

#endif //FENNEC_PARAMETER_H
