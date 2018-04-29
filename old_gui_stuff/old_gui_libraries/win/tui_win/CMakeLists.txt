# The name of our project is "TUI_WIN". CMakeLists files in this project can 
# refer to the root source directory of the project as ${TUI_WIN_SOURCE_DIR} and 
# to the root binary directory of the project as ${TUI_WIN_BINARY_DIR}. 
cmake_minimum_required (VERSION 2.4) 
project (TUI_WIN)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


include_directories (${TUI_WIN_SOURCE_DIR}/../../include)
include_directories (${TUI_WIN_SOURCE_DIR}/../../tui)

link_directories (${TUI_WIN_SOURCE_DIR}/../../)

add_library(TUI_WIN TuiWin.h TuiWin.cpp)

#target_link_libraries(TUI_WIN GUI)