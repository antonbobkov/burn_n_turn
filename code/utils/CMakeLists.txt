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

add_executable(file_utils_test file_utils_test.cpp)
target_link_libraries(file_utils_test PRIVATE Utils Catch2::Catch2WithMain)
add_test(NAME file_utils_test COMMAND $<TARGET_FILE:file_utils_test>)
