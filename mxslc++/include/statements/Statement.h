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
    using runtime_utils::Monomorphizable;
    using runtime_utils::RuntimeAware;

    class Statement : public Monomorphizable<StmtPtr>, public Stringable, protected RuntimeAware
    {
    public:
        explicit Statement(Token token) : token_{std::move(token)} { }
        ~Statement() override = default;

        const Token& token() const { return token_; }

        virtual void set_attributes(AttributeList attrs) { }

        StmtPtr monomorphize(const TypePtr& template_type) const override = 0;
        void execute();
        virtual void recover() const { }

        string to_string() const override = 0;

    protected:
        virtual void init() { }
        virtual void execute_impl() const = 0;

        Token token_;
        bool is_initialized_{false};

    private:
        void execute_and_locate_errors();
        bool is_recovery_point() const;

        bool has_failed_{false};
    };
}

#endif //FENNEC_STATEMENT_H
