//
// Created by jaket on 30/12/2025.
//

#include "load_mtlx.h"
#include <MaterialXFormat/Util.h>

#include "CompileError.h"
#include "runtime/Scope.h"
#include "expressions/NullExpression.h"
#include "runtime/Function.h"
#include "utils/io_utils.h"
#include "utils/str_utils.h"
#include "statements/Statement.h"

namespace
{
    Type to_type(const mx::TypeDefPtr& td)
    {
        return Type{td->getName()};
    }

    Parameter to_parameter(const Runtime& runtime, const mx::InputPtr& i)
    {
        const string& type = i->getType();
        const string& name = i->getName();
        ExprPtr expr = std::make_unique<NullExpression>(runtime);
        return Parameter{{}, type, name, std::move(expr)};
    }

    ParameterList get_parameters(const Runtime& runtime, const mx::NodeDefPtr& nd)
    {
        vector<Parameter> params;
        params.reserve(nd->getInputCount());
        for (mx::InputPtr i : nd->getInputs())
        {
            params.push_back(to_parameter(runtime, i));
        }

        return ParameterList{std::move(params)};
    }

    string get_type_name(const mx::NodeDefPtr& nd)
    {
        if (nd->getType() == "multioutput")
        {
            return "__"s + nd->getName() + "_return_type__"s;
        }
        else
        {
            return nd->getType();
        }
    }

    Function to_function(const Runtime& runtime, const mx::NodeDefPtr& nd)
    {
        const Scope& scope = runtime.scope();

        string type = get_type_name(nd);
        string name = nd->getNodeString();
        optional<string> template_type = get_postfix(name, '_');
        if (not scope.has_type(template_type.value()))
        {
            template_type = std::nullopt;
        }
        ParameterList params = get_parameters(runtime, nd);
        vector<StmtPtr> body;
        return Function{{}, std::move(type), std::move(name), template_type, std::move(params), std::move(body)};
    }
}

void load_library(const Runtime& runtime, const mx::DocumentPtr& doc)
{
    Scope& scope = runtime.scope();

    for (const mx::TypeDefPtr td : doc->getTypeDefs())
    {
        scope.add_type(to_type(td));
    }

    for (const mx::NodeDefPtr nd : doc->getNodeDefs())
    {
        if (nd->hasVersionString() and not nd->getDefaultVersion())
            continue;
        scope.add_function(to_function(runtime, nd));
    }
}

void load_library(const Runtime& runtime, const fs::path& filepath)
{
    const mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, filepath.string());
    load_library(runtime, doc);
}

void load_materialx_library(const Runtime& runtime, const string& version)
{
    const mx::FilePathVec fpv{version};
    const fs::path lib_dir = get_libraries_dir();
    const mx::FileSearchPath fsp{lib_dir.string()};
    const mx::DocumentPtr doc = mx::createDocument();
    const mx::StringSet loaded = mx::loadLibraries(fpv, fsp, doc);
    if (loaded.empty())
        throw CompileError{"Unsupported MaterialX version: "s + version};
    load_library(runtime, doc);
}
