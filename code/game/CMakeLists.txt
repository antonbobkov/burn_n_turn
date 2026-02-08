cmake_minimum_required(VERSION 3.14)
project(TowerDefense LANGUAGES CXX)

add_library(GameCore STATIC
  core.cpp
  entities.cpp
  level.cpp
  tutorial.cpp
  screen_controllers.cpp
  fireball.cpp
  critters.cpp
  critter_generators.cpp
  dragon.cpp
  gameplay.cpp
)
target_include_directories(GameCore PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/game_utils
  ${CMAKE_SOURCE_DIR}/wrappers
  ${CMAKE_SOURCE_DIR}/utils
)
target_link_libraries(GameCore PUBLIC GameUtils)

add_executable(tower_defense main.cpp)
target_include_directories(tower_defense PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/game_utils
  ${CMAKE_SOURCE_DIR}/wrappers_sdl_impl
  ${CMAKE_SOURCE_DIR}/wrappers
  ${CMAKE_SOURCE_DIR}/utils
  ${SDL_PATH}/include/SDL2
  ${SDL_IMAGE_PATH}/include/SDL2
  ${SDL_MIXER_PATH}/include/SDL2
)
target_link_directories(tower_defense PRIVATE
  ${SDL_PATH}/lib
  ${SDL_IMAGE_PATH}/lib
  ${SDL_MIXER_PATH}/lib
)
target_link_libraries(tower_defense PRIVATE
  GameCore
  GuiSdl
  SuiSdl
  mingw32
  SDL2main
  SDL2
  SDL2_image
  SDL2_mixer
)
