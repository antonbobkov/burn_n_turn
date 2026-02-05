cmake_minimum_required(VERSION 3.14)
project(Wrappers LANGUAGES CXX)

add_library(GuiGen
  GuiGenHdr.h
  GuiGen.h
  GuiGen.cpp
)

target_include_directories(GuiGen PRIVATE ${CMAKE_SOURCE_DIR}/utils)
target_link_libraries(GuiGen PRIVATE Utils)

add_library(SuiGen
  SuiGen.h
  SuiGen.cpp
)

target_include_directories(SuiGen PRIVATE ${CMAKE_SOURCE_DIR}/utils)
target_link_libraries(SuiGen PRIVATE Utils)
