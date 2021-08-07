cmake_minimum_required(VERSION 3.10)
project(detsvr LANGUAGES C CXX VERSION 0.1)

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