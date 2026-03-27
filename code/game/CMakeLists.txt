cmake_minimum_required(VERSION 3.14)
project(TowerDefense LANGUAGES CXX)

add_library(GameCore STATIC
  game_config.cc
  game_runner_interface_impl.cc
  dragon_game_runner.cc
  entities.cc
  level.cc
  tutorial.cc
  controller/entity_list_controller.cc
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
target_link_libraries(GameCore PUBLIC GameUtils)

add_executable(entity_list_controller_test controller/entity_list_controller_test.cc)
target_link_libraries(entity_list_controller_test PRIVATE GameCore GuiMock SuiMock Catch2::Catch2WithMain)
add_test(NAME entity_list_controller_test COMMAND $<TARGET_FILE:entity_list_controller_test>)
set_tests_properties(entity_list_controller_test PROPERTIES LABELS "unit")

add_executable(tower_defense
  ${CMAKE_SOURCE_DIR}/code/wrappers_sdl_impl/sdl_game_runner_main.cc)
target_include_directories(tower_defense PRIVATE
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
add_dependencies(tower_defense copy_sdl_dlls copy_bin_files)
