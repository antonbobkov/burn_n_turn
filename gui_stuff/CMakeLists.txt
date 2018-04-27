# The name of our project is "GUI". CMakeLists files in this project can 
# refer to the root source directory of the project as ${GUI_SOURCE_DIR} and 
# to the root binary directory of the project as ${GUI_BINARY_DIR}. 
cmake_minimum_required (VERSION 2.4) 
project (GUI)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

include_directories (${GUI_SOURCE_DIR}/include)

link_directories (${GUI_SOURCE_DIR})

add_library(GUI include/GuiGen.h include/GuiGenHdr.h src/GuiGen.cpp include/SuiGen.h src/SuiGen.cpp src/Preloader.cpp include/Preloader.h include/SmartPointer.h src/SmartPointer.cpp include/General.h src/General.cpp include/event.h src/event.cpp include/Global.h include/MessageWriter.h src/MessageWriter.cpp)