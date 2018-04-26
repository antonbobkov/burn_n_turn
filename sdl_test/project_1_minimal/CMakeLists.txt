cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0015 NEW)

project (sdl_minimal)

include_directories(../sdl_library_windows/all_inc)
link_directories(../sdl_library_windows/all_lib_64)

add_executable(sdl_minimal sdl_minimal.cpp)

target_link_libraries(sdl_minimal SDL2.lib SDL2main.lib)