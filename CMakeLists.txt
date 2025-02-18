cmake_minimum_required(VERSION 3.20)

project(paracl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PARACL_EXEC_NAME "${PROJECT_NAME}")
set(PARACL_SOURCE_PATH "${CMAKE_SOURCE_DIR}/paracl.cpp")

find_package(LLVM REQUIRED CONFIG)
if (LLVM_FOUND)
  message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
endif()

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

llvm_map_components_to_libnames(llvm_libs support core irreader)

find_package(GTest REQUIRED)
enable_testing()

find_package(Threads REQUIRED)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

set(LEXER_PATH "lib/parsing/lexer.l")
set(PARSER_PATH "lib/parsing/parser.y")

aux_source_directory(./lib/ast/ AST_SOURCES)
aux_source_directory(./lib/visitor/ VISITOR_SOURCES)
aux_source_directory(./lib/symbol_tab/ SYMBOL_TAB_SOURCES)

add_library(PARACL_INTERFACE_LIB INTERFACE)
target_include_directories(PARACL_INTERFACE_LIB INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

flex_target(scanner
  ${LEXER_PATH}
  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cc
  DEFINES_FILE
)

bison_target(parser
  ${PARSER_PATH}
  ${CMAKE_CURRENT_BINARY_DIR}/parser.cc
  DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/paracl_grammar.tab.hh)

add_flex_bison_dependency(scanner parser)

add_library(bison_parser STATIC ${BISON_parser_OUTPUTS} ${FLEX_scanner_OUTPUTS})
target_include_directories(bison_parser PUBLIC include/parsing/ ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(bison_parser PUBLIC include/ast/)
target_include_directories(bison_parser PUBLIC include/symbol_tab/)
target_include_directories(bison_parser PUBLIC include/visitor/)
target_include_directories(bison_parser PUBLIC include/codegen/)

add_compile_options(-Wall -Wextra -g)

set(INCLUDE_DIRS
  include/ast
  include/parsing
  include/visitor
  include/symbol_tab
  include/codegen
)

set(SOURCES
    ./lib/codegen.cpp
    ${AST_SOURCES}
    ${VISITOR_SOURCES}
    ${SYMBOL_TAB_SOURCES}
    ${BISON_parser_OUTPUTS}
    ${FLEX_scanner_OUTPUTS}
)

add_executable(${PARACL_EXEC_NAME}
  paracl.cpp
  ${SOURCES}
)
target_include_directories(${PARACL_EXEC_NAME} PUBLIC ${INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PARACL_EXEC_NAME} PUBLIC ${llvm_libs} bison_parser)

aux_source_directory(./tests/interactive INTERACTIVE_SOURCES)
aux_source_directory(./tests/unit UNIT_SOURCES)

set(TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests/)
  
list(APPEND TESTS "interactive" "unit")
list(APPEND TESTS_SOURCES "INTERACTIVE_SOURCES" "UNIT_SOURCES")

foreach(TEST TEST_SRC IN ZIP_LISTS TESTS TESTS_SOURCES)
  add_executable(${TEST} ${${TEST_SRC}} ${SOURCES})
  target_link_libraries(${TEST} PRIVATE ${GTEST_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${llvm_libs})
  target_include_directories(${TEST} PUBLIC ${INCLUDE_DIRS}
                                            ${CMAKE_CURRENT_BINARY_DIR} ${TESTS_DIR})
endforeach()

