cmake_minimum_required(VERSION 3.10)
project(uavvideo LANGUAGES C CXX VERSION 1.0)

find_package(OpenCV)
# include_directories(${OpenCV_INCLUDE_DIRS})

# cuda
# include_directories(/usr/local/cuda/include)
link_directories(/usr/local/cuda/lib64)
# tensorrt
# include_directories(/usr/include/aarch64-linux-gnu)
# link_directories(/usr/lib/aarch64-linux-gnu)
link_directories(${PROJECT_SOURCE_DIR}/../../build)

add_subdirectory(yolov5)

add_library(uavship SHARED uavship.cpp)
target_include_directories(uavship PUBLIC yolov5)
target_include_directories(uavship PRIVATE "/usr/local/cuda/include" "${OpenCV_INCLUDE_DIRS}")
target_link_libraries(uavship PUBLIC nvinfer)
target_link_libraries(uavship PUBLIC cudart)
target_link_libraries(uavship PUBLIC ${OpenCV_LIBS})
target_link_libraries(uavship PUBLIC libmyplugins.so)
set_property(TARGET uavship PROPERTY CXX_STANDARD 17)

add_executable(uavvideo run.cpp   
                        config.cpp
                        serialization.cpp                                            
)
target_include_directories(uavvideo PUBLIC "yolov5/"
                                           ${PROJECT_SOURCE_DIR}/../../detcore/include
                                           "/usr/local/cuda/include"
                                           "${OpenCV_INCLUDE_DIRS}"
)
target_link_libraries(uavvideo PRIVATE detcore)
target_link_libraries(uavvideo PUBLIC nvinfer)
target_link_libraries(uavvideo PUBLIC cudart)
target_link_libraries(uavvideo PUBLIC ${OpenCV_LIBS})
target_link_libraries(uavvideo PUBLIC myplugins)
set_property(TARGET uavvideo PROPERTY CXX_STANDARD 17)

add_executable(genconfig    genconfig.cpp
                            config.cpp
                            serialization.cpp
)
target_link_libraries(genconfig PRIVATE detcore)
set_property(TARGET genconfig PROPERTY CXX_STANDARD 17)