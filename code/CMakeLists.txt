# Configure and build all targets (from project root):
#   mkdir build
#   cd build
#   cmake ../code -G "MinGW Makefiles"
#   mingw32-make
#
# Builds Catch2 tests and SDL examples; example executables go into ../code/
#
cmake_minimum_required(VERSION 3.14)
project(Dragongame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Catch2 v2 (FetchContent)
include(FetchContent)
FetchContent_Declare(
  Catch2
  URL https://github.com/catchorg/Catch2/archive/v2.13.10.zip
)
set(CATCH_BUILD_STATIC_LIBRARY ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(Catch2)

# Absolute paths so subdirectories (sdl_examples) resolve includes correctly
set(SDL_PATH
    "${CMAKE_SOURCE_DIR}/../external_libraries/SDL2-2.32.10/i686-w64-mingw32"
    CACHE PATH "Path to SDL2" FORCE)
set(SDL_IMAGE_PATH
    "${CMAKE_SOURCE_DIR}/../external_libraries/SDL2_image-2.8.8/i686-w64-mingw32"
    CACHE PATH "Path to SDL2_image" FORCE)
set(SDL_MIXER_PATH
    "${CMAKE_SOURCE_DIR}/../external_libraries/SDL2_mixer-2.8.1/i686-w64-mingw32"
    CACHE PATH "Path to SDL2_mixer" FORCE)

# Build all example executables into the code directory (../code from build dir)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../bin/")

enable_testing()

# SDL examples (uses SDL_PATH, SDL_IMAGE_PATH, SDL_MIXER_PATH from above)
add_subdirectory(sdl_examples)

add_subdirectory(utils)

add_subdirectory(wrappers)

add_subdirectory(game_utils)

add_subdirectory(wrappers_sdl_impl)

add_subdirectory(game)