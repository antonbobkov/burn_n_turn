cmake_minimum_required(VERSION 3.14)
project(GameUtils LANGUAGES CXX)

add_library(GameUtils
  event.cpp
  event.h
  game_runner_interface.h
  MessageWriter.cpp
  MessageWriter.h
  Preloader.cpp
  Preloader.h
)

target_include_directories(GameUtils PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/utils
  ${CMAKE_SOURCE_DIR}/wrappers
)
target_link_libraries(GameUtils PRIVATE Utils GuiGen SuiGen)
