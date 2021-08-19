cmake_minimum_required(VERSION 3.10)
project(detsvr LANGUAGES C CXX VERSION 0.1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

add_subdirectory(test)

add_executable(detsvr src/run.cpp   src/base64.cpp 
                                    src/config.cpp
                                    src/detectionresult.cpp
                                    src/detectionservice.cpp
                                    src/utils.cpp
                                    src/plugincore.cpp
)
target_link_libraries(detsvr "pthread" "dl")
target_include_directories(detsvr PUBLIC "include/")
set_property(TARGET detsvr PROPERTY CXX_STANDARD 17)

add_executable(keygen src/keygen.cpp src/utils.cpp)
set_property(TARGET keygen PROPERTY CXX_STANDARD 17)

