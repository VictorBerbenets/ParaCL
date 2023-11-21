cmake_minimum_required(VERSION 3.15)

project(paracl)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

flex_target(scanner
  frontend/parsing/lexer.l
  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cc
)

bison_target(parser
  frontend/parsing/parser.y
  ${CMAKE_CURRENT_BINARY_DIR}/parser.cc
  COMPILE_FLAGS "--defines=${CMAKE_CURRENT_BINARY_DIR}/paracl_grammar.tab.hh -Wcounterexamples")

add_flex_bison_dependency(scanner parser)

add_executable(parser
  frontend/parsing/src/driver.cc
  ${BISON_parser_OUTPUTS}
  ${FLEX_scanner_OUTPUTS}
)

target_compile_features(parser PRIVATE cxx_std_20)
target_include_directories(parser PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(parser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/frontend/parsing/include)
target_include_directories(parser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/frontend/include/)
