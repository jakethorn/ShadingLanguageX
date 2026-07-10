add_library(mxslc_lib
        scan.cpp
        Token.cpp
        TokenReader.cpp
        parse.cpp
        compile.cpp
        TokenType.cpp
        Primitive.cpp
        errors/CompileError.cpp
        errors/AmbiguousFunctionError.cpp
        decompile/decompile.cpp
        decompile/Decompiler.cpp
        runtime/Runtime.cpp
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
        runtime/interface.cpp
        runtime/utils/monomorphize.cpp
        runtime/utils/RuntimeAware.cpp
        runtime/utils/invoke.cpp
        runtime/utils/type_cast.cpp
        runtime/utils/InitInfo.cpp
        runtime/utils/ArgumentEvaluator.cpp
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
        utils/string_utils.cpp
        utils/parse_cli_args.cpp
        utils/parse_utils.cpp
        utils/load_mtlx.cpp
        utils/mtlx_utils.cpp
        serialize/Serializer.cpp
        serialize/serialize_constexpr.cpp
        serialize/serializer_utils.cpp
        serialize/node_evaluators/binary_ops.cpp
        serialize/node_evaluators/separate.cpp
        serialize/node_evaluators/convert.cpp
        serialize/node_evaluators/combine.cpp
        serialize/node_evaluators/extract.cpp
        serialize/values/Value.cpp
        serialize/values/NodeValue.cpp
        serialize/values/InterfaceValue.cpp
        serialize/values/NodeOutputValue.cpp
        serialize/values/BasicValue.cpp
        serialize/values/NodeGraphValue.cpp
        serialize/values/NodeGraphOutputValue.cpp
        serialize/values/interface.cpp
        preprocess/preprocess.cpp
        preprocess/Macro.cpp
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
