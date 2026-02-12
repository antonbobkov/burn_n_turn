cmake_minimum_required(VERSION 3.14)
project(WrappersSDL LANGUAGES CXX)

add_library(GuiSdl
  GuiSdl.h
  GuiSdl.cc
)

target_include_directories(GuiSdl PRIVATE
  ${SDL_PATH}/include/SDL2
  ${SDL_IMAGE_PATH}/include/SDL2
  ${CMAKE_SOURCE_DIR}/wrappers
  ${CMAKE_SOURCE_DIR}/utils
)

target_link_directories(GuiSdl PRIVATE 
  ${SDL_PATH}/lib
  ${SDL_IMAGE_PATH}/lib
)

target_link_libraries(GuiSdl PRIVATE Utils GuiGen SDL2 SDL2_image)

add_library(SuiSdl
  SuiSdl.h
  SuiSdl.cc
)

target_include_directories(SuiSdl PRIVATE
  ${SDL_PATH}/include/SDL2
  ${SDL_MIXER_PATH}/include/SDL2
  ${CMAKE_SOURCE_DIR}/wrappers
  ${CMAKE_SOURCE_DIR}/utils
)

target_link_directories(SuiSdl PRIVATE
  ${SDL_PATH}/lib
  ${SDL_MIXER_PATH}/lib
)

target_link_libraries(SuiSdl PRIVATE Utils SuiGen SDL2 SDL2_mixer)

# gui_sdl_example: load and display image via SdlGraphicalInterface (like SDL_ex3_image)
add_executable(gui_sdl_example gui_sdl_example.cc)
target_include_directories(gui_sdl_example PRIVATE
  ${SDL_PATH}/include/SDL2
  ${SDL_IMAGE_PATH}/include/SDL2
  ${CMAKE_SOURCE_DIR}/wrappers
  ${CMAKE_SOURCE_DIR}/utils
)
target_link_directories(gui_sdl_example PRIVATE
  ${SDL_PATH}/lib
  ${SDL_IMAGE_PATH}/lib
)
target_link_libraries(gui_sdl_example PRIVATE GuiSdl mingw32 SDL2main SDL2 SDL2_image)

# sui_sdl_example: load and play music and sound via SuiSdl (like SDL_ex4_sound)
add_executable(sui_sdl_example sui_sdl_example.cc)
target_include_directories(sui_sdl_example PRIVATE
  ${SDL_PATH}/include/SDL2
  ${SDL_MIXER_PATH}/include/SDL2
  ${CMAKE_SOURCE_DIR}/wrappers
  ${CMAKE_SOURCE_DIR}/utils
)
target_link_directories(sui_sdl_example PRIVATE
  ${SDL_PATH}/lib
  ${SDL_MIXER_PATH}/lib
)
target_link_libraries(sui_sdl_example PRIVATE SuiSdl mingw32 SDL2main SDL2 SDL2_mixer)

# test_sdl: Catch2 tests for SDL loaders and SdlGraphicalInterface/SdlSoundInterface
add_executable(test_sdl test_sdl.cc)
target_include_directories(test_sdl PRIVATE
  ${SDL_PATH}/include/SDL2
  ${SDL_IMAGE_PATH}/include/SDL2
  ${SDL_MIXER_PATH}/include/SDL2
  ${CMAKE_SOURCE_DIR}/wrappers
  ${CMAKE_SOURCE_DIR}/utils
)
target_link_directories(test_sdl PRIVATE
  ${SDL_PATH}/lib
  ${SDL_IMAGE_PATH}/lib
  ${SDL_MIXER_PATH}/lib
)
target_link_libraries(test_sdl PRIVATE Catch2::Catch2WithMain GuiSdl SuiSdl mingw32 SDL2main SDL2 SDL2_image SDL2_mixer)

add_test(NAME test_sdl COMMAND $<TARGET_FILE:test_sdl>)
set_tests_properties(test_sdl PROPERTIES WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
