cmake_minimum_required(VERSION 3.14)
project(WrappersMock LANGUAGES CXX)

add_library(GuiMock GuiMock.h GuiMock.cpp)
target_include_directories(GuiMock PRIVATE
  ${CMAKE_SOURCE_DIR}/wrappers_mock
  ${CMAKE_SOURCE_DIR}/wrappers
  ${CMAKE_SOURCE_DIR}/utils
)
target_link_libraries(GuiMock PRIVATE GuiGen Utils)

add_library(SuiMock SuiMock.h SuiMock.cpp)
target_include_directories(SuiMock PRIVATE
  ${CMAKE_SOURCE_DIR}/wrappers
  ${CMAKE_SOURCE_DIR}/utils
)
target_link_libraries(SuiMock PRIVATE SuiGen Utils)

add_executable(wrappers_mock_test wrappers_mock_test.cpp)
target_include_directories(wrappers_mock_test PRIVATE
  ${CMAKE_SOURCE_DIR}/wrappers_mock
  ${CMAKE_SOURCE_DIR}/wrappers
  ${CMAKE_SOURCE_DIR}/utils
)
target_link_libraries(wrappers_mock_test PRIVATE
  Catch2::Catch2WithMain GuiMock SuiMock Utils
)
add_test(NAME wrappers_mock_test COMMAND $<TARGET_FILE:wrappers_mock_test>)
