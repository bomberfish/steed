cmake_minimum_required(VERSION 3.5)
project(Steed)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
    GIT_SHALLOW 1
)

FetchContent_MakeAvailable(raylib)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/sources/*.cpp")
set(PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/sources/")

add_executable(${PROJECT_NAME})

add_definitions(-DPHYSAC_IMPLEMENTATION)

if (EMSCRIPTEN)
  target_compile_options(${PROJECT_NAME} PRIVATE -DEMSCRIPTEN)
  target_link_options(${PROJECT_NAME} PRIVATE --shell-file "${CMAKE_CURRENT_SOURCE_DIR}/webport/index.html" --preload-file "${CMAKE_CURRENT_SOURCE_DIR}/assets@/" -sINITIAL_MEMORY=70254592 -sASYNCIFY -sEXPORTED_FUNCTIONS=_main)
  set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE INTERNAL "")
  target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="./")
else()
  target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="./assets/")
endif()

target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Add clangd to the src directory")