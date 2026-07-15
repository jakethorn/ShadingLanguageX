//
// Created by jaket on 06/11/2025.
//

#ifndef FENNEC_STATEMENT_H
#define FENNEC_STATEMENT_H

#include "Token.h"
#include "runtime/AttributeList.h"
#include "runtime/utils/RuntimeAware.h"
#include "common.h"

namespace mxslc::statements
{
    using runtime_utils::RuntimeAware;
    using runtime_utils::Monomorphizable;

    class Statement : protected RuntimeAware, public Monomorphizable<StmtPtr>, public Stringable
    {
    public:
        explicit Statement(Token token) : token_{std::move(token)} { }
        ~Statement() override = default;

        const Token& token() const { return token_; }

        virtual void set_attributes(AttributeList attrs) { }

        StmtPtr monomorphize(const TypePtr& template_type) const override = 0;
        virtual void init() { }
        void execute();

        string to_string() const override = 0;

    protected:
        virtual void execute_impl() const = 0;

        Token token_;
        bool is_initialized_{false};
    };
}

#endif //FENNEC_STATEMENT_H
