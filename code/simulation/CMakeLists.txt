cmake_minimum_required(VERSION 3.14)
project(Simulation LANGUAGES CXX)

add_library(Simulation simulation.cc)
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

add_executable(simulation_test simulation_test.cc)
target_include_directories(simulation_test PRIVATE
  ${CMAKE_SOURCE_DIR}/game
  ${CMAKE_SOURCE_DIR}/game_utils
  ${CMAKE_SOURCE_DIR}/utils
  ${CMAKE_SOURCE_DIR}/wrappers
  ${CMAKE_SOURCE_DIR}/wrappers_mock
)
target_link_libraries(simulation_test PRIVATE
  Simulation Catch2::Catch2WithMain
)
add_test(NAME simulation_test COMMAND $<TARGET_FILE:simulation_test>)
set_tests_properties(simulation_test PROPERTIES
  WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
