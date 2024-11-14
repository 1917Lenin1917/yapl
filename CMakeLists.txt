cmake_minimum_required(VERSION 3.29)
project(yapl)

set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_SOURCE_DIR}/includes)

add_executable(yapl src/main.cpp
        src/Lexer.cpp
        src/Token.cpp
        src/Parser.cpp
        src/Parser.hpp)

add_subdirectory(externals/Catch2)
add_executable(tests
        tests/test_lexer.cpp src/Token.cpp src/Lexer.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
