# The name of our project is "TOWER". CMakeLists files in this project can 
# refer to the root source directory of the project as ${TOWER_SOURCE_DIR} and 
# to the root binary directory of the project as ${TOWER_BINARY_DIR}. 
cmake_minimum_required (VERSION 2.4) 
project (TOWER)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

if(UNIX)
    include_directories (${TOWER_SOURCE_DIR}/include_ln)
else(UNIX)
    include_directories (${TOWER_SOURCE_DIR}/include)
endif(UNIX)

link_directories (${TOWER_SOURCE_DIR})

add_executable(TOWER main.cpp GuiGen.h GuiGen.cpp GuiSdl.cpp GuiSdl.h noisyfly.h Preloader.cpp Preloader.h SmartPointer.h SmartPointer.cpp tower_defense.h tower_defense.cpp)

target_link_libraries(TOWER SDLmain SDL SDL_image SDL_mixer)
