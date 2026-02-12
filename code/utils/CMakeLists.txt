cmake_minimum_required(VERSION 3.14)
project(Utils LANGUAGES CXX)

add_library(Utils
  debug_log.h
  debug_log.cc
  exception.h
  exception.cc
  file_utils.h
  file_utils.cc
  timer.h
  timer.cc
  index.h
  index.cc
  General.h
  General.cc
  smart_pointer.h
  smart_pointer.cc
)

add_executable(file_utils_test file_utils_test.cc)
target_link_libraries(file_utils_test PRIVATE Utils Catch2::Catch2WithMain)
add_test(NAME file_utils_test COMMAND $<TARGET_FILE:file_utils_test>)

add_executable(smart_pointer_test smart_pointer_test.cc)
target_link_libraries(smart_pointer_test PRIVATE Utils Catch2::Catch2WithMain)
add_test(NAME smart_pointer_test COMMAND $<TARGET_FILE:smart_pointer_test>)
