cmake_minimum_required(VERSION 3.10)
project(videoplayer LANGUAGES C CXX VERSION 1.0)

add_executable(videoplayer  run.cpp 
                            config.cpp
                            serialization.cpp
)
target_link_libraries(videoplayer PRIVATE detcore)
set_property(TARGET videoplayer PROPERTY CXX_STANDARD 17)