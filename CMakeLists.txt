cmake_minimum_required(VERSION 3.10)
project(glass LANGUAGES CXX VERSION 0.0.1)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_STANDARD 17)

set(SOURCES
    ${SRC_DIR}/parser.hpp ${SRC_DIR}/parser.cpp
    ${SRC_DIR}/lexer.hpp ${SRC_DIR}/lexer.cpp
    ${SRC_DIR}/backend.hpp ${SRC_DIR}/backend.cpp
)

add_executable(glassc ${SOURCES} ${SRC_DIR}/compiler.cpp)
add_executable(glass-repl ${SOURCES} ${SRC_DIR}/repl.cpp)
