cmake_minimum_required(VERSION 3.29)
project(yapl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
include_directories(${CMAKE_SOURCE_DIR}/includes)
#set(CMAKE_BUILD_TYPE "RelWithDebInfo")
#set(CMAKE_CXX_FLAGS "-g -O0 -fno-omit-frame-pointer -gdwarf-2")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
#set(CMAKE_CXX_FLAGS "-O3")

add_executable(yapl src/main.cpp
        src/Lexer.cpp
        src/Token.cpp
        src/Parser.cpp
        src/Interpreter.cpp
        src/ASTNode.cpp
        src/Scope.cpp
        src/Variable.cpp
        src/Function.cpp
        src/Visitor.cpp
        src/Values/Value.cpp
        src/Values/IntegerValue.cpp
        src/Values/BooleanValue.cpp
        src/Values/FloatValue.cpp
        src/Values/StringValue.cpp
        src/Values/ArrayValue.cpp
)

add_subdirectory(externals/Catch2)
add_executable(tests
        tests/test_lexer.cpp src/Token.cpp src/Lexer.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
