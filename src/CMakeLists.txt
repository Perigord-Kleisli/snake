add_executable(${PROJECT_NAME} Main.c)

include_directories(${PROJECT_NAME} ${SDL2_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} m)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()


if(EMSCRIPTEN)
  execute_process(COMMAND emcc --cflags
      OUTPUT_VARIABLE EM_CFLAGS)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${EM_CFLAGS}")
  include_directories(${PROJECT_SOURCE_DIR}/include)
endif()
