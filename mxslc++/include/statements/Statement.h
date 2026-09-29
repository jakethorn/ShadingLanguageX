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
        // the statement that this statement is a copy of, e.g., in the body of a templated function, see monomorphize
        const Statement& origin() const { return origin_ ? *origin_ : *this; }
        void set_origin(const Statement& stmt) { origin_ = &stmt.origin(); }

        virtual void set_attributes(AttributeList attrs) { }

        StmtPtr monomorphize(const TypePtr& template_type) const override = 0;
        void execute();

        string to_string() const override = 0;
        // statements with a body, e.g., functions and loops, are separated from other statements by an empty line
        virtual bool is_block() const { return false; }

        // statements that are recorded by the decompile hints, see serialize/decompile_hints.h
        virtual bool is_hinted() const { return false; }
        // the statement without the parts that can be recovered from the graph, called after the statement is executed
        virtual string hint_skeleton() const { return ""; }

    protected:
        static string with_attributes(const AttributeList& attrs, const string& statement);

        virtual void init() { }
        virtual void execute_impl() const = 0;

        Token token_;
        bool is_initialized_{false};

    private:
        const Statement* origin_{nullptr};
    };

    string join_statements(const vector<StmtPtr>& statements);
}

#endif //FENNEC_STATEMENT_H
