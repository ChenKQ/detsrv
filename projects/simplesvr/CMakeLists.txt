cmake_minimum_required(VERSION 3.10)
project(simplesvr LANGUAGES C CXX VERSION 1.0)

add_executable(simplesvr simplesvr.cpp  service.cpp
                                        config.cpp
)
# target_include_directories(simplesvr PRIVATE   
#                            ${PROJECT_SOURCE_DIR}/../detcore/include)
target_link_libraries(simplesvr PRIVATE detcore)
set_property(TARGET simplesvr PROPERTY CXX_STANDARD 17)
