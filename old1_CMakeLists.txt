# The name of our project is "Total". CMakeLists files in this project can 
# refer to the root source directory of the project as ${Total_SOURCE_DIR} and 
# to the root binary directory of the project as ${Total_BINARY_DIR}. 
cmake_minimum_required (VERSION 2.4) 
project (Total)

SET (GUI_ABS_PATH "${Total_SOURCE_DIR}/gui_stuff")

add_subdirectory(gui_stuff)
add_subdirectory(gui_stuff/sdl/tui_sdl)
add_subdirectory(gui_stuff/win/tui_win)
add_subdirectory(gui_stuff/win/nui_win)
add_subdirectory(gui_stuff/sdl)
add_subdirectory(dragongame/pluanbo/tower_defense)