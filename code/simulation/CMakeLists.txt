cmake_minimum_required(VERSION 3.14)
project(Simulation LANGUAGES CXX)

add_library(Simulation simulation.cpp)
target_include_directories(Simulation PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/game
  ${CMAKE_SOURCE_DIR}/game_utils
  ${CMAKE_SOURCE_DIR}/utils
  ${CMAKE_SOURCE_DIR}/wrappers
  ${CMAKE_SOURCE_DIR}/wrappers_mock
)
target_link_libraries(Simulation PUBLIC
  GameCore
  GuiMock
  SuiMock
  Utils
  GameUtils
)

add_executable(simulation_main simulation_main.cpp)
target_link_libraries(simulation_main PRIVATE Simulation)
