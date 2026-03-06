cmake_minimum_required(VERSION 3.14)
project(Dragongame LANGUAGES CXX)

add_subdirectory(sdl_examples)

add_subdirectory(utils)

add_subdirectory(wrappers)

add_subdirectory(game_utils)

add_subdirectory(wrappers_sdl_impl)

add_subdirectory(wrappers_mock)

add_subdirectory(game)

add_subdirectory(simulation)