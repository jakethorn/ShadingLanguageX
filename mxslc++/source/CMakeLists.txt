add_library(mxslc_lib
        scan.cpp
        Token.cpp
        TokenReader.cpp
        parse.cpp
        compile.cpp
        CompileError.cpp
        runtime/Scope.cpp
        runtime/Argument.cpp
        runtime/ArgumentList.cpp
        runtime/Function.cpp
        runtime/Parameter.cpp
        runtime/ParameterList.cpp
        runtime/Variable.cpp
        runtime/Type.cpp
        expressions/Expression.cpp
        expressions/FunctionCall.cpp
        expressions/Identifier.cpp
        expressions/Literal.cpp
        expressions/ExpressionFactory.cpp
        expressions/NullExpression.cpp
        expressions/UnnamedConstructor.cpp
        expressions/IndexingExpression.cpp
        statements/VariableDefinition.cpp
        statements/PrintStatement.cpp
        statements/FunctionDefinition.cpp
        statements/MultiVariableDefinition.cpp
        utils/io_utils.cpp
        utils/str_utils.cpp
        utils/parse_args.cpp
        mtlx/load_mtlx.cpp
        mtlx/mtlx_utils.cpp
        mtlx/MtlXSerializer.cpp
        values/ValueFactory.cpp
)

target_include_directories(mxslc_lib
        PUBLIC  "${CMAKE_SOURCE_DIR}/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(mxslc_lib
    PUBLIC mtlx_lib
)
