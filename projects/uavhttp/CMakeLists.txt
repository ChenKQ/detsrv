cmake_minimum_required(VERSION 3.10)
project(uavhttp LANGUAGES C CXX VERSION 1.0)

find_package(OpenCV)
# include_directories(${OpenCV_INCLUDE_DIRS})

# cuda
include_directories(/usr/local/cuda/include)
link_directories(/usr/local/cuda/lib64)
# tensorrt
include_directories(/usr/include/x86_64-linux-gnu/)
link_directories(/usr/lib/x86_64-linux-gnu/)
link_directories(${PROJECT_SOURCE_DIR}/../../build)

add_subdirectory(yolov5)

add_executable(uavhttp run.cpp   config.cpp
                                 serialization.cpp
                                 detectionservice.cpp
                                 uavship.cpp 

)
target_include_directories(uavhttp PUBLIC "yolov5/"
                                           ${PROJECT_SOURCE_DIR}/../../detcore/include
                                           "/usr/local/cuda/include"
                                           "${OpenCV_INCLUDE_DIRS}"
)
target_link_libraries(uavhttp PRIVATE detcore)
target_link_libraries(uavhttp PUBLIC nvinfer)
target_link_libraries(uavhttp PUBLIC cudart)
target_link_libraries(uavhttp PUBLIC ${OpenCV_LIBS})
target_link_libraries(uavhttp PUBLIC myplugins)
set_property(TARGET uavhttp PROPERTY CXX_STANDARD 17)