cmake_minimum_required(VERSION 3.14)
project(WrappersMock LANGUAGES CXX)

add_library(GuiMock GuiMock.h GuiMock.cc)
target_link_libraries(GuiMock PRIVATE GuiGen Utils)

add_library(SuiMock SuiMock.h SuiMock.cc)
target_link_libraries(SuiMock PRIVATE SuiGen Utils)

add_executable(wrappers_mock_test wrappers_mock_test.cc)
target_link_libraries(wrappers_mock_test PRIVATE
  Catch2::Catch2WithMain GuiMock SuiMock Utils
)
add_test(NAME wrappers_mock_test COMMAND $<TARGET_FILE:wrappers_mock_test>)
set_tests_properties(wrappers_mock_test PROPERTIES LABELS "unit")
