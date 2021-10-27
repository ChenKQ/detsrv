cmake_minimum_required(VERSION 3.10)
project(multitask LANGUAGES C CXX VERSION 1.0)

find_package(OpenCV)
# include_directories(${OpenCV_INCLUDE_DIRS})

# cuda
# include_directories(/usr/local/cuda/include)
link_directories(/usr/local/cuda/lib64)
# tensorrt
# include_directories(/usr/include/aarch64-linux-gnu)
# link_directories(/usr/lib/aarch64-linux-gnu)
link_directories(${PROJECT_SOURCE_DIR}/../../build)

add_subdirectory(yolopcut)

add_library(inference SHARED inference.cpp)
target_include_directories(inference PUBLIC yolop)
target_include_directories(inference PRIVATE "/usr/local/cuda/include" "${OpenCV_INCLUDE_DIRS}")
target_link_libraries(inference PUBLIC nvinfer)
target_link_libraries(inference PUBLIC cudart)
target_link_libraries(inference PUBLIC ${OpenCV_LIBS})
target_link_libraries(inference PUBLIC libmyplugins.so)
set_property(TARGET inference PROPERTY CXX_STANDARD 17)

add_executable(multitask run.cpp   
                        config.cpp
                        serialization.cpp   
)
target_include_directories(multitask PUBLIC "yolop/"
                                           ${PROJECT_SOURCE_DIR}/../../detcore/include
                                           "/usr/local/cuda/include"
                                           "${OpenCV_INCLUDE_DIRS}"
)
target_link_libraries(multitask PRIVATE detcore)
target_link_libraries(multitask PUBLIC nvinfer)
target_link_libraries(multitask PUBLIC cudart)
target_link_libraries(multitask PUBLIC ${OpenCV_LIBS})
target_link_libraries(multitask PUBLIC myplugins)
set_property(TARGET multitask PROPERTY CXX_STANDARD 17)

add_executable(genconfig    genconfig.cpp
                            config.cpp
                            serialization.cpp
)
target_link_libraries(genconfig PRIVATE detcore)
set_property(TARGET genconfig PROPERTY CXX_STANDARD 17)