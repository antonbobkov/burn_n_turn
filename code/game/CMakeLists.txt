cmake_minimum_required(VERSION 3.14)
project(TowerDefense LANGUAGES CXX)

add_library(GameCore STATIC
  game_runner_interface_impl.cc
  dragon_game_runner.cc
  entities.cc
  level.cc
  tutorial.cc
  controller/basic_controllers.cc
  controller/buy_now_controller.cc
  controller/dragon_game_controller.cc
  controller/level_controller.cc
  controller/menu_controller.cc
  fireball.cc
  critters.cc
  critter_generators.cc
  dragon.cc
)
target_include_directories(GameCore PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(GameCore PUBLIC GameUtils)

add_executable(tower_defense
  ${CMAKE_SOURCE_DIR}/wrappers_sdl_impl/sdl_game_runner_main.cc)
target_include_directories(tower_defense PRIVATE
  ${CMAKE_SOURCE_DIR}
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
