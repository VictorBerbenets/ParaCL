cmake_minimum_required(VERSION 3.15)

project(paracl)

set(CMAKE_CXX_STANDARD 20)

find_package(GTest REQUIRED)
enable_testing()

find_package(Threads REQUIRED)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

aux_source_directory(./frontend/ast/src AST_SOURCES)
aux_source_directory(./frontend/examples EXAMPLES)
aux_source_directory(./frontend/visitor/src VISITOR_SOURCES)
aux_source_directory(./frontend/symbol_tab/src SYMBOL_TAB_SOURCES)

flex_target(scanner
  frontend/parsing/lexer.l
  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cc
  DEFINES_FILE
)

bison_target(parser
  frontend/parsing/parser.y
  ${CMAKE_CURRENT_BINARY_DIR}/parser.cc
  DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/paracl_grammar.tab.hh)

add_flex_bison_dependency(scanner parser)

add_library(bison_parser STATIC ${BISON_parser_OUTPUTS} ${FLEX_scanner_OUTPUTS})
target_include_directories(bison_parser PUBLIC frontend/parsing/include ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(bison_parser PUBLIC frontend/ast/)
target_include_directories(bison_parser PUBLIC frontend/symbol_tab/)
target_include_directories(bison_parser PUBLIC frontend/visitor/include)

add_executable(paraCL
  frontend/parsing/src/driver.cc
  ${AST_SOURCES}
  ${VISITOR_SOURCES}
  ${SYMBOL_TAB_SOURCES}
  ${BISON_parser_OUTPUTS}
  ${FLEX_scanner_OUTPUTS}
)

add_executable(example
  examples/main.cpp
  examples/examples.cpp
  ${AST_SOURCES}
  ${VISITOR_SOURCES}
  ${SYMBOL_TAB_SOURCES}
  ${BISON_parser_OUTPUTS}
  ${FLEX_scanner_OUTPUTS}
)

set_target_properties(
  paraCL PROPERTIES
    COMPILE_FLAGS " -Wall -Wextra "
)

target_link_libraries(paraCL PUBLIC bison_parser)
target_link_libraries(example PUBLIC bison_parser)
target_link_libraries(example PRIVATE ${GTEST_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

target_include_directories(paraCL PUBLIC frontend/include frontend/parsing/include ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(example PUBLIC frontend/include frontend/parsing/include ${CMAKE_CURRENT_BINARY_DIR})
