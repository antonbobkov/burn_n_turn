# The name of our project is "GUI_SDL". CMakeLists files in this project can 
# refer to the root source directory of the project as ${GUI_SDL_SOURCE_DIR} and 
# to the root binary directory of the project as ${GUI_SDL_BINARY_DIR}. 
cmake_minimum_required (VERSION 2.4) 
project (GUI_SDL)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

if(UNIX)
	include_directories (${GUI_SDL_SOURCE_DIR}/include_ln)
else(UNIX)
	include_directories (${GUI_SDL_SOURCE_DIR}/include)
endif(UNIX)

include_directories (${GUI_SDL_SOURCE_DIR}/../include)

link_directories (${GUI_SDL_SOURCE_DIR}/lib)
link_directories (${GUI_SDL_SOURCE_DIR}/../)

add_library(GUI_SDL GuiSdl.h GuiSdl.cpp SuiSdl.h SuiSdl.cpp)

target_link_libraries(GUI_SDL GUI SDLmain SDL SDL_image SDL_mixer)