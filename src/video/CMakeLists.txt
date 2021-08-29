find_package(OpenCV)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(videoanalysis videoanalysis.cpp 
                             videoreader.cpp
                             playmanager.cpp
                             rtmpwriter.cpp
                            ../plugincore.cpp 
                            ../config.cpp
                            ../utils.cpp
)
target_include_directories(videoanalysis PRIVATE ${OpenCV_INCLUDE_DIRS} 
                                                "../../include"
)
target_link_libraries(videoanalysis PRIVATE ${OpenCV_LIBS} "dl" "pthread")

add_executable(videoplay videoplay.cpp videoreader.cpp playmanager.cpp)
target_include_directories(videoplay PRIVATE ${OpenCV_INCLUDE_DIRS} 
                                                "../../include"
)
target_link_libraries(videoplay PRIVATE ${OpenCV_LIBS} "pthread")