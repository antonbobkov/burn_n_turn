cmake_minimum_required(VERSION 3.14)
project(SdlExamples LANGUAGES CXX)


# SDL examples: built as console apps so a terminal opens and logs (cout/cerr)
# are visible. To run in WIN32 mode (no terminal): add WIN32 to add_executable(),
# e.g. add_executable(SDL_ex1_basic WIN32 SDL_ex1_basic.cc)

# SDL_ex1_basic: simple window with cyan background
add_executable(SDL_ex1_basic SDL_ex1_basic.cc)
target_include_directories(SDL_ex1_basic PRIVATE ${SDL_PATH}/include/SDL2)
target_link_directories(SDL_ex1_basic PRIVATE ${SDL_PATH}/lib)
target_link_libraries(SDL_ex1_basic PRIVATE mingw32 SDL2main SDL2)

# SDL_ex2_interactive: physics demo with user interaction
add_executable(SDL_ex2_interactive SDL_ex2_interactive.cc)
target_include_directories(SDL_ex2_interactive PRIVATE ${SDL_PATH}/include/SDL2)
target_link_directories(SDL_ex2_interactive PRIVATE ${SDL_PATH}/lib)
target_link_libraries(SDL_ex2_interactive PRIVATE mingw32 SDL2main SDL2)

# SDL_ex3_image: display image from file
add_executable(SDL_ex3_image SDL_ex3_image.cc)
target_include_directories(SDL_ex3_image PRIVATE
    ${SDL_PATH}/include/SDL2
    ${SDL_IMAGE_PATH}/include/SDL2)
target_link_directories(SDL_ex3_image PRIVATE ${SDL_PATH}/lib ${SDL_IMAGE_PATH}/lib)
target_link_libraries(SDL_ex3_image PRIVATE mingw32 SDL2main SDL2 SDL2_image)

# SDL_ex4_sound: load and play music and sound effects
add_executable(SDL_ex4_sound SDL_ex4_sound.cc)
target_include_directories(SDL_ex4_sound PRIVATE
    ${SDL_PATH}/include/SDL2
    ${SDL_MIXER_PATH}/include/SDL2)
target_link_directories(SDL_ex4_sound PRIVATE ${SDL_PATH}/lib ${SDL_MIXER_PATH}/lib)
target_link_libraries(SDL_ex4_sound PRIVATE mingw32 SDL2main SDL2 SDL2_mixer)

add_dependencies(SDL_ex1_basic copy_sdl_dlls)
add_dependencies(SDL_ex2_interactive copy_sdl_dlls)
add_dependencies(SDL_ex3_image copy_sdl_dlls)
add_dependencies(SDL_ex4_sound copy_sdl_dlls)
