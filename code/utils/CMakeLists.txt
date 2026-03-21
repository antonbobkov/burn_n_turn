cmake_minimum_required(VERSION 3.14)
project(Utils LANGUAGES CXX)

add_library(Utils
  configuration_file.h
  configuration_file.cc
  exception.h
  exception.cc
  file_utils.h
  file_utils.cc
  timer.h
  timer.cc
  index.h
  index.cc
  math_utils.h
  random_utils.h
  random_utils.cc
  string_utils.h
  string_utils.cc
)


add_executable(configuration_file_test configuration_file_test.cc)
target_link_libraries(configuration_file_test PRIVATE Utils Catch2::Catch2WithMain)
add_test(NAME configuration_file_test COMMAND $<TARGET_FILE:configuration_file_test>)
set_tests_properties(configuration_file_test PROPERTIES LABELS "unit")

add_executable(file_utils_test file_utils_test.cc)
target_link_libraries(file_utils_test PRIVATE Utils Catch2::Catch2WithMain)
add_test(NAME file_utils_test COMMAND $<TARGET_FILE:file_utils_test>)
set_tests_properties(file_utils_test PROPERTIES LABELS "unit")
