cmake_minimum_required(VERSION 3.20)

project(paracl)

set(CMAKE_CXX_STANDARD 20)

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
aux_source_directory(./lib/semantics/ SEMANTICS_SOURCES)

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

add_subdirectory(include)

add_library(bison_parser STATIC ${BISON_parser_OUTPUTS} ${FLEX_scanner_OUTPUTS})
target_include_directories(bison_parser PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(bison_parser PUBLIC ParaclHeaders)

add_compile_options(-Wall -Wextra)

set(SOURCES
    ./lib/codegen.cpp
    ./lib/option_category.cpp
    ${AST_SOURCES}
    ${VISITOR_SOURCES}
    ${SEMANTICS_SOURCES}
    ${BISON_parser_OUTPUTS}
    ${FLEX_scanner_OUTPUTS}
)

add_executable(${PARACL_EXEC_NAME}
  paracl.cpp
  ${SOURCES}
)

target_include_directories(${PARACL_EXEC_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PARACL_EXEC_NAME} PUBLIC ParaclHeaders ${llvm_libs} bison_parser)
