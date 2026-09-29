//
// Created by jaket on 01/09/2026.
//

#ifndef MXSLC_RETURNSTATEMENT_H
#define MXSLC_RETURNSTATEMENT_H

#include "statements/Statement.h"

namespace mxslc::statements
{
    class ReturnStatement final : public Statement
    {
    public:
        explicit ReturnStatement(ExprPtr expr, Token token = {});

        void set_attributes(AttributeList attrs) override;

        StmtPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;
        const ExprPtr& expression() const { return expr_; }

        bool is_hinted() const override { return true; }
        string hint_skeleton() const override;

    protected:
        void execute_impl() const override;

    private:
        ExprPtr expr_;
        // the returned value as it is written in the skeleton, see HintRecorder::expression_skeleton
        mutable string hint_value_{"_"};

    public:
        class Branch : public std::exception
        {
        public:
            explicit Branch(VarPtr return_value = nullptr) : return_value_{std::move(return_value)} { }

            const char* what() const noexcept override
            {
                return "internal error: 'return' branch escaped the function body";
            }

            VarPtr return_value() const { return return_value_; }

        private:
            VarPtr return_value_;
        };
    };
}

#endif //MXSLC_RETURNSTATEMENT_H
