cmake_minimum_required(VERSION 3.27)

project(snake)

if(EMSCRIPTEN)
    set(USE_FLAGS "-s USE_SDL=2")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXECUTABLE_SUFFIX .html)
else()
    find_package(SDL2 REQUIRED)
endif()


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)
