//
// Created by jaket on 19/06/2026.
//

#include <MaterialXFormat/XmlIo.h>

#include "decompile/Decompiler.h"
#include "decompile/DocumentDecompiler.h"
#include "decompile/DocumentHints.h"
#include "decompile/GraphComparator.h"
#include "compile.h"
#include "CompileOptions.h"
#include "constants.h"
#include "errors/CompileError.h"
#include "utils/io_utils.h"
#include "utils/Logger.h"
#include "utils/load_mtlx.h"
#include "utils/string_utils.h"

namespace mxslc::decompile
{
    using string_utils::starts_with;

    namespace
    {
        vector<int> version_numbers(const string& version)
        {
            vector<int> numbers;
            size_t start = 0;
            while (start < version.size())
            {
                size_t end = version.find('.', start);
                if (end == string::npos)
                    end = version.size();
                numbers.push_back(std::atoi(version.substr(start, end - start).c_str()));
                start = end + 1;
            }
            return numbers;
        }

        // documents only store the major and minor version, e.g., 1.39, so the latest available patch version is used
        string find_library_version(const string& document_version, const vector<fs::path>& search_directories)
        {
            const string prefix = document_version + ".";
            if (document_version.empty() or starts_with(DEFAULT_MTLX_VERSION, prefix))
                return DEFAULT_MTLX_VERSION;

            string best_version;
            for (const fs::path& directory : search_directories)
            {
                const fs::path libraries_dir = directory / "libraries";
                std::error_code error;
                if (not fs::is_directory(libraries_dir, error))
                    continue;

                for (const fs::directory_entry& entry : fs::directory_iterator{libraries_dir, error})
                {
                    const string version = entry.path().filename().string();
                    if (entry.is_directory() and starts_with(version, prefix) and version_numbers(version) > version_numbers(best_version))
                        best_version = version;
                }
            }

            return best_version.empty() ? DEFAULT_MTLX_VERSION : best_version;
        }

        mx::ElementPtr find_element(const mx::DocumentPtr& document, const mx::ElementPtr& element, const string& type_name)
        {
            if (element == nullptr)
                throw CompileError{"Cannot decompile null " + type_name};
            if (element->getDocument() == document)
                return element;
            // the element belongs to the original document, find its copy
            if (mx::ElementPtr copy = document->getDescendant(element->getNamePath()))
                return copy;
            throw CompileError{"Cannot find " + type_name + ": " + element->getNamePath()};
        }
    }

    Decompiler::Decompiler(const fs::path& src_path) : document_{mx::createDocument()}
    {
        mx::readFromXmlFile(document_, src_path.string());
        load_data_library(io_utils::get_default_search_directories(src_path));
    }

    Decompiler::Decompiler(const string& source) : document_{mx::createDocument()}
    {
        mx::readFromXmlString(document_, source);
        load_data_library(io_utils::get_default_search_directories());
    }

    Decompiler::Decompiler(const mx::DocumentPtr& document) : document_{mx::createDocument()}
    {
        document_->copyContentFrom(document);
        load_data_library(io_utils::get_default_search_directories());
    }

    void Decompiler::load_data_library(const vector<fs::path>& search_directories)
    {
        // the node defs of the standard library are needed to know the order and default values of node inputs
        library_version_ = find_library_version(document_->getVersionString(), search_directories);
        try
        {
            document_->setDataLibrary(load_materialx_library(library_version_, search_directories));
        }
        catch (const CompileError&)
        {
            // without the libraries, nodes are decompiled as function calls with named arguments
        }
    }

    string Decompiler::decompile_document()
    {
        // hints can be inconsistent with the graph, e.g., after the graph was edited, in ways that are not detected, so
        // the code created with them is only used if it compiles to the same graph, otherwise less of them are used
        string hinted_code;
        if (not DocumentHints{document_}.empty())
        {
            for (const HintUsage hint_usage : {HintUsage::All, HintUsage::WithoutLibraryCalls, HintUsage::WithoutCalls})
            {
                const string code = decompile_document(hint_usage);
                if (not code.empty() and is_equivalent(code))
                    return code;
                if (hinted_code.empty())
                    hinted_code = code;
            }
            Logger::debug("Decompile hints were ignored, because the code created with them does not compile to the same graph.");
        }

        string code = decompile_document(HintUsage::None);
        if (hinted_code.empty() or is_equivalent(code))
            return code;

        // neither is known to be correct, so the code that is closer to the original code is used
        return hinted_code;
    }

    string Decompiler::decompile_document(const HintUsage hint_usage) const
    {
        try
        {
            DocumentDecompiler decompiler{document_, hint_usage};
            string code = decompiler.decompile_document();

            // values used after their variable was assigned a new value are copied to another variable first
            if (not decompiler.stale_values().empty())
            {
                DocumentDecompiler snapshot_decompiler{document_, hint_usage};
                snapshot_decompiler.set_snapshots(decompiler.stale_values());
                code = snapshot_decompiler.decompile_document();
            }
            return code;
        }
        catch (const std::exception&)
        {
            // decompiling without hints never fails because of hints
            if (hint_usage == HintUsage::None)
                throw;
            return "";
        }
    }

    bool Decompiler::is_equivalent(const string& code) const
    {
        try
        {
            CompileOptions opts;
            opts.version = library_version_;
            opts.reduce_graph = false;
            opts.validate_graph = false;
            const mx::DocumentPtr recompiled = compile_to_document(code, opts);
            return GraphComparator::differences(document_, recompiled).empty();
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    string Decompiler::decompile_node(const string& node_name, const bool with_dependencies)
    {
        return decompile_node(document_->getNode(node_name), with_dependencies);
    }

    string Decompiler::decompile_node(const mx::NodePtr& node, const bool with_dependencies)
    {
        const mx::NodePtr copy = find_element(document_, node, "Node")->asA<mx::Node>();
        return DocumentDecompiler{document_, HintUsage::None}.decompile_node(copy, with_dependencies);
    }

    string Decompiler::decompile_node_def(const string& node_def_name, const bool with_dependencies)
    {
        return decompile_node_def(document_->getNodeDef(node_def_name), with_dependencies);
    }

    string Decompiler::decompile_node_def(const mx::NodeDefPtr& node_def, const bool with_dependencies)
    {
        const mx::ElementPtr copy = find_element(document_, node_def, "NodeDef");
        return DocumentDecompiler{document_, HintUsage::None}.decompile_function(copy, with_dependencies);
    }

    string Decompiler::decompile_node_graph(const string& node_graph_name, const bool with_dependencies)
    {
        return decompile_node_graph(document_->getNodeGraph(node_graph_name), with_dependencies);
    }

    string Decompiler::decompile_node_graph(const mx::NodeGraphPtr& node_graph, const bool with_dependencies)
    {
        const mx::NodeGraphPtr copy = find_element(document_, node_graph, "NodeGraph")->asA<mx::NodeGraph>();

        // node graphs that implement a node def are decompiled as the function of the node def
        if (const mx::NodeDefPtr node_def = copy->getNodeDef(); node_def and node_def->getDocument() == document_)
            return DocumentDecompiler{document_, HintUsage::None}.decompile_function(node_def, with_dependencies);

        return DocumentDecompiler{document_, HintUsage::None}.decompile_function(copy, with_dependencies);
    }
}
