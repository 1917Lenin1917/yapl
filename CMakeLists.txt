cmake_minimum_required(VERSION 3.29)
project(yapl)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_FLAGS "-O3")
include_directories(${CMAKE_SOURCE_DIR}/includes)
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>")

add_executable(yapl src/main.cpp
        src/Lexer.cpp
        src/Token.cpp
        src/Parser.cpp
        src/Interpreter.cpp
        src/ASTNode.cpp
        src/Value.cpp
        src/Scope.cpp
        src/Variable.cpp
        src/Function.cpp
        src/Visitor.cpp
)

add_subdirectory(externals/Catch2)
add_executable(tests
        tests/test_lexer.cpp src/Token.cpp src/Lexer.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
