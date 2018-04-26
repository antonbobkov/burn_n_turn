cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0015 NEW)

project (sdl_sound)

include_directories(../sdl_library_windows/all_inc)
link_directories(../sdl_library_windows/all_lib_64)

add_executable(sdl_sound 21_sound_effects_and_music.cpp)

target_link_libraries(sdl_sound SDL2.lib SDL2main.lib SDL2_mixer.lib SDL2_image.lib)