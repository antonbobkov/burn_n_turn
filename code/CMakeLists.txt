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

# SDL2: fetch pre-built MinGW dev zip (use lib/ and include/, do not build from source)
set(SDL2_VERSION "2.32.10" CACHE STRING "SDL2 release version")
set(SDL2_ARCH "i686-w64-mingw32" CACHE STRING "MinGW arch: i686-w64-mingw32 or x86_64-w64-mingw32")
FetchContent_Declare(
  SDL2
  URL "https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-devel-${SDL2_VERSION}-mingw.zip"
)
FetchContent_Populate(SDL2)
FetchContent_GetProperties(SDL2 SOURCE_DIR SDL2_SRC)
# Zip extracts with i686-w64-mingw32/ and x86_64-w64-mingw32/ underneath (each has include/SDL2, lib)
set(SDL_PATH "${SDL2_SRC}/${SDL2_ARCH}" CACHE PATH "Path to SDL2 (pre-built include + lib)" FORCE)

# SDL2_image: fetch pre-built MinGW dev zip (same layout as SDL2)
set(SDL2_IMAGE_VERSION "2.8.8" CACHE STRING "SDL2_image release version")
FetchContent_Declare(
  SDL2_image
  URL "https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMAGE_VERSION}/SDL2_image-devel-${SDL2_IMAGE_VERSION}-mingw.zip"
)
FetchContent_Populate(SDL2_image)
FetchContent_GetProperties(SDL2_image SOURCE_DIR SDL2_IMAGE_SRC)
set(SDL_IMAGE_PATH "${SDL2_IMAGE_SRC}/${SDL2_ARCH}" CACHE PATH "Path to SDL2_image (pre-built include + lib)" FORCE)

# SDL2_mixer: fetch pre-built MinGW dev zip (same layout as SDL2)
set(SDL2_MIXER_VERSION "2.8.1" CACHE STRING "SDL2_mixer release version")
FetchContent_Declare(
  SDL2_mixer
  URL "https://github.com/libsdl-org/SDL_mixer/releases/download/release-${SDL2_MIXER_VERSION}/SDL2_mixer-devel-${SDL2_MIXER_VERSION}-mingw.zip"
)
FetchContent_Populate(SDL2_mixer)
FetchContent_GetProperties(SDL2_mixer SOURCE_DIR SDL2_MIXER_SRC)
set(SDL_MIXER_PATH "${SDL2_MIXER_SRC}/${SDL2_ARCH}" CACHE PATH "Path to SDL2_mixer (pre-built include + lib)" FORCE)

# Build all example executables into the code directory (../code from build dir)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../bin/")

enable_testing()

# SDL examples (uses SDL_PATH, SDL_IMAGE_PATH, SDL_MIXER_PATH from above)
add_subdirectory(sdl_examples)

add_subdirectory(utils)

add_subdirectory(wrappers)

add_subdirectory(game_utils)

add_subdirectory(wrappers_sdl_impl)

add_subdirectory(wrappers_mock)

add_subdirectory(game)

add_subdirectory(simulation)