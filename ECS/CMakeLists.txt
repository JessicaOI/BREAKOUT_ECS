cmake_minimum_required(VERSION 3.28)
project(ECS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules)

# Configuración de SDL2
set(SDL2_PATH "C:/Users/jessi/OneDrive/Escritorio/New folder/sdl/SDL2-devel-2.30.5-mingw/SDL2-2.30.5/x86_64-w64-mingw32")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

# Configuración para incluir entt mediante FetchContent
include(FetchContent)
FetchContent_Declare(
        entt
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG v3.11.1  # Cambia a la última versión disponible si es necesario
)
FetchContent_MakeAvailable(entt)

# Crear el ejecutable
add_executable(ECS main.cpp)

# Incluir los directorios de entt
target_include_directories(${PROJECT_NAME} PRIVATE ${entt_SOURCE_DIR}/src)

# Enlazar bibliotecas SDL2 y entt
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY} EnTT::EnTT)
