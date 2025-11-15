cmake_minimum_required(VERSION 3.10)
project(glass LANGUAGES CXX VERSION 0.0.1)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_STANDARD 17)

find_path(READLINE_INCLUDE_DIR
  NAMES readline/readline.h
  HINTS /usr/local/include /usr/include
)

find_library(READLINE_LIBRARY
  NAMES readline
  HINTS /usr/local/lib /usr/lib
)


set(SOURCES
    ${SRC_DIR}/parser.hpp ${SRC_DIR}/parser.cpp
    ${SRC_DIR}/lexer.hpp ${SRC_DIR}/lexer.cpp
    ${SRC_DIR}/backend.hpp ${SRC_DIR}/backend.cpp
)

add_executable(glassc ${SOURCES} ${SRC_DIR}/compiler.cpp)
add_executable(glass ${SOURCES} ${SRC_DIR}/interpreter.cpp)

if (READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
    message(STATUS "found readline: ${READLINE_LIBRARY}")
    include_directories(${READLINE_INCLUDE_DIR})
    target_link_libraries(glass ${READLINE_LIBRARY})
    target_compile_definitions(glass PRIVATE GLASS_USE_READLINE)
else()
    message(WARNING "readline not found, provide readline path by setting CMake options READLINE_LIBRARY and READLINE_INCLUDE_DIR")
endif()
