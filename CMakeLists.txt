cmake_minimum_required(VERSION 3.15)

project(paracl)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

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
target_include_directories(bison_parser PUBLIC frontend/include frontend/parsing/include ${CMAKE_CURRENT_BINARY_DIR})

add_executable(driver
  frontend/parsing/src/driver.cc
  ${BISON_parser_OUTPUTS}
  ${FLEX_scanner_OUTPUTS}
)

target_link_libraries(driver PUBLIC bison_parser)
target_include_directories(driver PUBLIC frontend/include frontend/parsing/include ${CMAKE_CURRENT_BINARY_DIR})

# target_compile_features(parser PRIVATE cxx_std_20)
# target_include_directories(parser PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
# target_include_directories(parser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/frontend/parsing/include)
# target_include_directories(parser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/frontend/include/)
