add_library(mxslc_lib
        scan.cpp
        Token.cpp
        TokenReader.cpp
        parse.cpp
        compile.cpp
        errors/CompileError.cpp
        errors/AmbiguousFunctionError.cpp
        decompile/decompile.cpp
        decompile/Decompiler.cpp
        runtime/Runtime.cpp
        runtime/RuntimeAccessor.cpp
        runtime/RuntimeUtils.cpp
        runtime/Scope.cpp
        runtime/Argument.cpp
        runtime/ArgumentList.cpp
        runtime/Function.cpp
        runtime/FunctionQuery.cpp
        runtime/Parameter.cpp
        runtime/ParameterList.cpp
        runtime/Type.cpp
        runtime/Field.cpp
        runtime/ModifierList.cpp
        runtime/Variable.cpp
        runtime/InputVariable.cpp
        runtime/Attribute.cpp
        runtime/AttributeList.cpp
        expressions/Expression.cpp
        expressions/FunctionCall.cpp
        expressions/Identifier.cpp
        expressions/Literal.cpp
        expressions/ExpressionFactory.cpp
        expressions/NullExpression.cpp
        expressions/UnnamedConstructor.cpp
        expressions/IndexingOperator.cpp
        expressions/DotOperator.cpp
        expressions/VariableDefinitionExpression.cpp
        expressions/accessors/PortAccessor.cpp
        expressions/accessors/FieldAccessor.cpp
        expressions/accessors/ComponentAccessor.cpp
        expressions/IncrementOperator.cpp
        expressions/NamedConstructor.cpp
        expressions/ThisExpression.cpp
        expressions/MethodCall.cpp
        expressions/CompoundAssignment.cpp
        expressions/RuntimeExpression.cpp
        expressions/TypeOfOperator.cpp
        expressions/RangeExpression.cpp
        statements/Statement.cpp
        statements/VariableDefinition.cpp
        statements/PrintStatement.cpp
        statements/FunctionDefinition.cpp
        statements/MultiVariableDefinition.cpp
        statements/UsingDeclaration.cpp
        statements/VariableAssignment.cpp
        statements/ForEachLoop.cpp
        statements/ExpressionStatement.cpp
        statements/IfStatement.cpp
        statements/BlockStatement.cpp
        statements/DocumentAttribute.cpp
        statements/ClassDefinition.cpp
        statements/ConstructorDefinition.cpp
        utils/io_utils.cpp
        utils/str_utils.cpp
        utils/parse_args.cpp
        utils/instantiate_template_types_utils.cpp
        utils/parse_utils.cpp
        utils/type_cast.cpp
        utils/InitInfo.cpp
        utils/ArgumentEvaluator.cpp
        mtlx/load_mtlx.cpp
        mtlx/mtlx_utils.cpp
        mtlx/MtlXSerializer.cpp
        mtlx/evaluate_mtlx.cpp
        mtlx/node_evaluators/binary_ops.cpp
        mtlx/node_evaluators/separate.cpp
        mtlx/node_evaluators/convert.cpp
        mtlx/node_evaluators/combine.cpp
        mtlx/node_evaluators/extract.cpp
        mtlx/node_evaluators/invert.cpp
        values/Value.cpp
        values/ValueFactory.cpp
        values/NodeValue.cpp
        values/InterfaceValue.cpp
        values/NodeOutputValue.cpp
        values/BasicValue.cpp
        values/NodeGraphValue.cpp
        values/NodeGraphOutputValue.cpp
        interface/Variable.cpp
)

set_target_properties(mxslc_lib PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

target_include_directories(mxslc_lib
        PUBLIC  "${CMAKE_SOURCE_DIR}/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(mxslc_lib
        PUBLIC mtlx_lib
)

if(UNIX AND NOT APPLE)
    target_link_libraries(mxslc_lib
            PUBLIC dl
    )
endif()
