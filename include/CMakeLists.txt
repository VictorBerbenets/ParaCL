
add_library(ParaclHeaders INTERFACE)

target_include_directories(ParaclHeaders
  INTERFACE
  parsing
  ast
  visitor
  codegen
  symbol_tab
  semantics
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )
