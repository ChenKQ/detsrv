cmake_minimum_required(VERSION 3.10)
project(detsvr LANGUAGES C CXX VERSION 0.1)

set(LIBRARY_OUTPUT_PATH    ${PROJECT_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -Wl,-rpath=./")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wl,-rpath=./")

add_subdirectory(detcore)
add_subdirectory(tools)

set(PRONAME "multitask")
add_subdirectory(projects/${PRONAME})