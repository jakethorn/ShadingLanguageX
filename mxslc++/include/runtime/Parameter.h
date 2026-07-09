//
// Created by jaket on 07/11/2025.
//

#ifndef FENNEC_PARAMETER_H
#define FENNEC_PARAMETER_H

#include "runtime/AttributeList.h"
#include "runtime/ModifierList.h"
#include "common.h"
#include "utils/RuntimeAware.h"

namespace mxslc::runtime
{
    class Parameter : protected runtime_utils::RuntimeAware
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
        TypePtr type() const;
        const string& name() const { return name_; }
        size_t index() const { return index_; }

        Parameter monomorphize(const TypePtr& template_type) const;
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
