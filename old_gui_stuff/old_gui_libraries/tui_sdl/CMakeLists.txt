# The name of our project is "TUI_SDL". CMakeLists files in this project can 
# refer to the root source directory of the project as ${TUI_SDL_SOURCE_DIR} and 
# to the root binary directory of the project as ${TUI_SDL_BINARY_DIR}. 
cmake_minimum_required (VERSION 2.4) 
project (TUI_SDL)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

if(UNIX)
	include_directories (${TUI_SDL_SOURCE_DIR}/../include_ln)
else(UNIX)
	include_directories (${TUI_SDL_SOURCE_DIR}/../include)
endif(UNIX)

include_directories (${TUI_SDL_SOURCE_DIR}/../../include)
include_directories (${TUI_SDL_SOURCE_DIR}/../../tui)

link_directories (${TUI_SDL_SOURCE_DIR}/../lib)
link_directories (${TUI_SDL_SOURCE_DIR}/../../)

add_library(TUI_SDL TuiSdl.h TuiSdl.cpp)

target_link_libraries(TUI_SDL SDLmain SDL)