cmake_minimum_required(VERSION 3.14)
project(Utils LANGUAGES CXX)

add_library(Utils
  debug_log.h
  debug_log.cpp
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
  smart_pointer.h
  smart_pointer.cpp
)

add_executable(file_utils_test file_utils_test.cpp)
target_link_libraries(file_utils_test PRIVATE Utils Catch2::Catch2WithMain)
add_test(NAME file_utils_test COMMAND $<TARGET_FILE:file_utils_test>)

add_executable(smart_pointer_test smart_pointer_test.cpp)
target_link_libraries(smart_pointer_test PRIVATE Utils Catch2::Catch2WithMain)
add_test(NAME smart_pointer_test COMMAND $<TARGET_FILE:smart_pointer_test>)
