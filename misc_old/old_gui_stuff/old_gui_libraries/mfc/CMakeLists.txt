# The name of our project is "GUI_MFC". CMakeLists files in this project can 
# refer to the root source directory of the project as ${GUI_MFC_SOURCE_DIR} and 
# to the root binary directory of the project as ${GUI_MFC_BINARY_DIR}. 
cmake_minimum_required (VERSION 2.4) 
project (GUI_MFC)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

if(WIN32)

	SET(CMAKE_MFC_FLAG 2)

	include_directories (${GUI_MFC_SOURCE_DIR}/../include)
	link_directories (${GUI_MFC_SOURCE_DIR}/../)

	add_library(GUI_MFC GuiMfc.h GuiMfc.cpp SuiMfc.h SuiMfc.cpp)

	target_link_libraries(GUI_MFC GUI)

endif(WIN32)

