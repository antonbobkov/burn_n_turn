# The name of our project is "NUI_WIN". CMakeLists files in this project can 
# refer to the root source directory of the project as ${NUI_WIN_SOURCE_DIR} and 
# to the root binary directory of the project as ${NUI_WIN_BINARY_DIR}. 
cmake_minimum_required (VERSION 2.4) 

if(WIN32)

	project (NUI_WIN)

	if(COMMAND cmake_policy)
	  cmake_policy(SET CMP0003 NEW)
	endif(COMMAND cmake_policy)

	include_directories (${NUI_WIN_SOURCE_DIR}/../../include)
	include_directories (${NUI_WIN_SOURCE_DIR}/../tui_win)
	include_directories (${NUI_WIN_SOURCE_DIR}/../../nui)
	include_directories (${NUI_WIN_SOURCE_DIR}/../../tui)

	link_directories (${NUI_WIN_SOURCE_DIR}/../../)
	link_directories (${NUI_WIN_SOURCE_DIR}/../tui_win)

	#add_executable(NUI_WIN NuiWin.h NuiWin.cpp main.cpp)
	add_library(NUI_WIN NuiWin.h NuiWin.cpp)

	target_link_libraries(NUI_WIN TUI_WIN GUI ws2_32)

endif(WIN32)
