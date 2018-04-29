# CMakeLists files in this project can refer to the root source directory of the project as ${<project_name>_SOURCE_DIR} and to the root binary directory of the project as ${<project_name>_BINARY_DIR}.

cmake_minimum_required (VERSION 2.6) 
cmake_policy(SET CMP0015 NEW)

project (Dragon)

SET (SDL_DIR "${Dragon_SOURCE_DIR}/sdl_library_windows")
SET (MyUtilLibrary_DIR "${Dragon_SOURCE_DIR}/gui_src")

SET (SDL_LIBRARY_LIST "SDL2 SDL2main SDL2_mixer SDL2_image")

include_directories (${SDL_DIR}/all_inc)
link_directories (${SDL_DIR}/all_lib_64)

add_library(MyUtilLibrary
	${MyUtilLibrary_DIR}/SuiGen.cpp
	${MyUtilLibrary_DIR}/Preloader.cpp
	${MyUtilLibrary_DIR}/Preloader.h
	${MyUtilLibrary_DIR}/SmartPointer.h
	${MyUtilLibrary_DIR}/SmartPointer.cpp
	${MyUtilLibrary_DIR}/General.h
	${MyUtilLibrary_DIR}/General.cpp
	${MyUtilLibrary_DIR}/event.h
	${MyUtilLibrary_DIR}/event.cpp
	${MyUtilLibrary_DIR}/Global.h
	${MyUtilLibrary_DIR}/MessageWriter.h
	${MyUtilLibrary_DIR}/MessageWriter.cpp
	${MyUtilLibrary_DIR}/GuiGen.h
	${MyUtilLibrary_DIR}/GuiGenHdr.h
	${MyUtilLibrary_DIR}/GuiGen.cpp
	${MyUtilLibrary_DIR}/SuiGen.h
	${MyUtilLibrary_DIR}/SuiGen.cpp
	${MyUtilLibrary_DIR}/GuiSdl.h
	${MyUtilLibrary_DIR}/GuiSdl.cpp
	${MyUtilLibrary_DIR}/SuiSdl.h
	${MyUtilLibrary_DIR}/SuiSdl.cpp
	)
	
# target_link_libraries(MyUtilLibrary ${SDL_LIBRARY_LIST})

include_directories (${MyUtilLibrary_DIR})
add_library(Game
	main.cpp
	tower_defense.h
	tower_defense.cpp
	)
	
target_link_libraries(Game MyUtilLibrary)

# add_executable(sdl_sound 21_sound_effects_and_music.cpp)

