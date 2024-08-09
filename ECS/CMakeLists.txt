cmake_minimum_required(VERSION 3.28)
project(untitled)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules)

set(SDL2_PATH "C:/Users/jessi/OneDrive/Escritorio/New folder/sdl/SDL2-devel-2.30.5-mingw/SDL2-2.30.5/x86_64-w64-mingw32")

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

add_executable(untitled main.cpp)

# Link SDL2 and SDL2_ttf libraries along with necessary Windows system libraries
target_link_libraries(untitled ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES} "${SDL2_PATH}/lib/libSDL2.a" "${SDL2_PATH}/lib/libSDL2main.a" setupapi imm32 version winmm)
