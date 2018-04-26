cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0015 NEW)

project (sdl_image)

include_directories(../sdl_library_windows/all_inc)
link_directories(../sdl_library_windows/all_lib_64)

add_executable(sdl_image 02_getting_an_image_on_the_screen.cpp)

target_link_libraries(sdl_image SDL2.lib SDL2main.lib)