cmake_minimum_required(VERSION 3.29)
project(yapl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -g -march=native -fno-signed-zeros -fno-trapping-math -frename-registers -funroll-loops")
include_directories(${CMAKE_SOURCE_DIR}/includes)
#set(CMAKE_BUILD_TYPE "RelWithDebInfo")
#set(CMAKE_CXX_FLAGS "-g -O0 -fno-omit-frame-pointer -gdwarf-2")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
#set(CMAKE_CXX_FLAGS "-O3")

find_package(OpenSSL REQUIRED COMPONENTS Crypto)

add_executable(yapl src/main.cpp
        src/Lexer.cpp
        src/Token.cpp
        src/Parser.cpp
        src/ASTNode.cpp
        src/Variable.cpp
        src/values/Value.cpp
        src/values/IntegerValue.cpp
        src/values/BooleanValue.cpp
        src/values/FloatValue.cpp
        src/values/StringValue.cpp
        src/values/ArrayValue.cpp
        src/exceptions/SyntaxError.cpp
        src/exceptions/RuntimeError.cpp
        src/values/TypeObjectValue.cpp
        src/values/DictValue.cpp
        src/values/UserDefinedValue.cpp
        src/values/FunctionValue.cpp
        src/values/SizeIterator.cpp
        includes/yapl/ByteCode.hpp
        includes/yapl/ByteCodeVM.hpp
        src/ByteCodeVM.cpp
        includes/yapl/ByteCodeVisitor.hpp
        includes/yapl/CodeObject.hpp
        src/ByteCodeVisitor.cpp
        src/values/UndefinedValue.cpp
        includes/yapl/values/UndefinedValue.hpp
        src/values/CodeObjectValue.cpp
        includes/yapl/values/CodeObjectValue.hpp
        src/values/BuiltinFunctionValue.cpp
        src/CodeObject.cpp
        includes/yapl/Serialization.hpp
        src/Serialization.cpp
        includes/yapl/Hasher.hpp
)

add_subdirectory(externals/Catch2)
add_executable(tests
        tests/test_lexer.cpp
        tests/test_parser.cpp

        src/ByteCodeVM.cpp
        src/ByteCodeVisitor.cpp
        src/Lexer.cpp
        src/Token.cpp
        src/Parser.cpp
        src/ASTNode.cpp
        src/Variable.cpp

        src/values/Value.cpp
        src/values/CodeObjectValue.cpp
        src/values/UndefinedValue.cpp
        src/values/SizeIterator.cpp
        src/values/StringValue.cpp
        src/values/FloatValue.cpp
        src/values/BooleanValue.cpp
        src/values/IntegerValue.cpp
        src/values/ArrayValue.cpp
        src/values/BuiltinFunctionValue.cpp
        src/values/DictValue.cpp
        src/values/FunctionValue.cpp
        src/values/TypeObjectValue.cpp

        src/exceptions/SyntaxError.cpp
        src/exceptions/RuntimeError.cpp
        src/exceptions/StopIteration.cpp
)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
target_link_libraries(yapl PRIVATE OpenSSL::Crypto)

#set_target_properties(yapl PROPERTIES UNITY_BUILD ON)