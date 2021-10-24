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

add_executable(uavloop  run.cpp   
                        config.cpp
                        serialization.cpp                                            
)
target_include_directories(uavloop PUBLIC "yolov5/"
                                           ${PROJECT_SOURCE_DIR}/../../detcore/include
                                           "/usr/local/cuda/include"
                                           "${OpenCV_INCLUDE_DIRS}"
)
target_link_libraries(uavloop PRIVATE detcore)
target_link_libraries(uavloop PUBLIC nvinfer)
target_link_libraries(uavloop PUBLIC cudart)
target_link_libraries(uavloop PUBLIC ${OpenCV_LIBS})
target_link_libraries(uavloop PUBLIC myplugins)
set_property(TARGET uavloop PROPERTY CXX_STANDARD 17)