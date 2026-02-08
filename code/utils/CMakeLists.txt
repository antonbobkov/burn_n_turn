cmake_minimum_required(VERSION 3.14)
project(Utils LANGUAGES CXX)

add_library(Utils
  exception.h
  exception.cpp
  file_utils.h
  file_utils.cpp
  timer.h
  timer.cpp
  index.h
  index.cpp
  General.h
  General.cpp
  SmartPointer.h
  SmartPointer.cpp
)
