//
// Created by jaket on 03/01/2026.
//

#ifndef FENNEC_FUNCTIONDEFINITION_H
#define FENNEC_FUNCTIONDEFINITION_H

#include "utils/common.h"
#include "Statement.h"
#include "Token.h"
#include "runtime/Function.h"
#include "runtime/ParameterList.h"

class FunctionDefinition final : public Statement
{
public:
    FunctionDefinition(const Runtime& runtime, vector<string> modifiers, Type type, Token name,
                        vector<Type> template_types, ParameterList params, vector<StmtPtr> body)
                            : Statement{runtime}, modifiers_{std::move(modifiers)}, type_{std::move(type)},
                            name_{std::move(name)}, template_types_{std::move(template_types)},
                            params_{std::move(params)}, body_{std::move(body)} { }

    StmtPtr instantiate_templated_types(const Type& template_type) const override;
    void init() override;
    void execute() override;

private:
    [[nodiscard]] bool is_templated() const { return not template_types_.empty(); }

    vector<string> modifiers_;
    Type type_;
    Token name_;
    vector<Type> template_types_;
    ParameterList params_;
    vector<StmtPtr> body_;

    vector<Function> funcs_;
};

#endif //FENNEC_FUNCTIONDEFINITION_H
