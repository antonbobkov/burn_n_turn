# The name of our project is "TOWER". CMakeLists files in this project can 
# refer to the root source directory of the project as ${TOWER_SOURCE_DIR} and 
# to the root binary directory of the project as ${TOWER_BINARY_DIR}. 
cmake_minimum_required (VERSION 2.4) 
project (TOWER)


if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

include_directories (${GUI_ABS_PATH}/include)

add_library(TOWER tower_defense.h tower_defense.cpp)

target_link_libraries(TOWER GUI)

# The name of our project is "TOWER_SDL". CMakeLists files in this project can 
# refer to the root source directory of the project as ${TOWER_SDL_SOURCE_DIR} and 
# to the root binary directory of the project as ${TOWER_SDL_BINARY_DIR}. 
cmake_minimum_required (VERSION 2.4) 
project (TOWER_SDL)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

if(UNIX)
	include_directories (${GUI_ABS_PATH}/sdl/include_ln)
else(UNIX)
	include_directories (${GUI_ABS_PATH}/sdl/include)
endif(UNIX)

include_directories (${GUI_ABS_PATH}/include)
include_directories (${GUI_ABS_PATH}/sdl)
include_directories (${GUI_ABS_PATH}/sdl/default/)

link_directories (${GUI_ABS_PATH})
link_directories (${GUI_ABS_PATH}/sdl/)
link_directories (${GUI_ABS_PATH}/sdl/lib)

if(APPLE)
	add_executable(TOWER_SDL ${GUI_ABS_PATH}/sdl/default/main.cpp ${GUI_ABS_PATH}/sdl/default/SDLMain.m)
else(APPLE)
	add_executable(TOWER_SDL ${GUI_ABS_PATH}/sdl/default/main.cpp )
endif(APPLE)

target_link_libraries(TOWER_SDL TOWER GUI_SDL)

#set_target_properties(TOWER_SDL PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")