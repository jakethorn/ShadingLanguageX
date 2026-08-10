//
// Created by jaket on 07/11/2025.
//

#ifndef FENNEC_ARGUMENT_H
#define FENNEC_ARGUMENT_H

#include "common.h"
#include "Primitive.h"
#include "runtime/AttributeList.h"
#include "runtime/ModifierList.h"
#include "runtime/utils/monomorphize.h"

namespace mxslc::runtime
{
    using runtime_utils::Monomorphizable;

    class Parameter;

    class Argument : public Monomorphizable<Argument>, public Stringable
    {
    public:
        Argument(AttributeList attrs, ModifierList mods, string name, ExprPtr expr, size_t index);
        Argument(ModifierList mods, string name, ExprPtr expr, size_t index);
        Argument(ModifierList mods, ExprPtr expr, size_t index);
        Argument(string name, ExprPtr expr, size_t index);

        Argument(ExprPtr expr, size_t index);
        Argument(VarPtr value, size_t index);
        Argument(ValuePtr value, size_t index);
        Argument(Primitive value, size_t index);

        Argument(Argument&& other) noexcept;

        ~Argument() override;

        const AttributeList& attributes() const { return attrs_; }
        const ModifierList& modifiers() const { return mods_; }
        bool has_name() const { return not name_.empty(); }
        const string& name() const { return name_; }
        size_t index() const { return index_; }

        Argument monomorphize(const TypePtr& template_type) const override;

        void init(const TypePtr& type) const;
        void init(const vector<TypePtr>& types) const;
        bool try_init(const vector<TypePtr>& types) const;
        void update() const;
        void reset() const;

        bool is_initialized() const;
        bool is_literal() const;

        TypePtr type() const;
        VarPtr evaluate() const;

        void validate(const Parameter& param) const;

        const string& error_message() const;

        string to_string() const override;

    private:
        AttributeList attrs_;
        ModifierList mods_;
        string name_;
        ExprPtr expr_;
        size_t index_;
    };
}

#endif //FENNEC_ARGUMENT_H
