# The name of our project is "TUI". CMakeLists files in this project can 
# refer to the root source directory of the project as ${TUI_SOURCE_DIR} and 
# to the root binary directory of the project as ${TUI_BINARY_DIR}. 
cmake_minimum_required (VERSION 2.4) 

project (TUI_GEN)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_executable(TUI_GEN TemplateGenerator.cpp)