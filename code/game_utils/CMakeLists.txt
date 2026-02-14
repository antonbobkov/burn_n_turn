cmake_minimum_required(VERSION 3.14)
project(GameUtils LANGUAGES CXX)

add_library(GameUtils
  draw_utils.cc
  event.cc
  event.h
  game_runner_interface.h
  image_sequence.cc
  image_sequence.h
  MessageWriter.cc
  MessageWriter.h
  mouse_utils.cc
  Preloader.cc
  Preloader.h
  sound_sequence.cc
  sound_sequence.h
  sound_utils.cc
)

target_include_directories(GameUtils PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(GameUtils PRIVATE Utils GuiGen SuiGen)
