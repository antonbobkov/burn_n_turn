cmake_minimum_required(VERSION 3.14)
project(Utils LANGUAGES CXX)

add_library(Utils
  General.h
  General.cpp
  SmartPointer.h
  SmartPointer.cpp
)
