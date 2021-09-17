cmake_minimum_required(VERSION 3.10)
project(detsvr LANGUAGES C CXX VERSION 0.1)

set(LIBRARY_OUTPUT_PATH    ${PROJECT_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

# add_subdirectory(detcore)
# add_subdirectory(tools)
add_subdirectory(tests)

# add_executable(detsvr src/run.cpp   src/config.cpp
#                                     src/serialization.cpp
#                                     src/detectionservice.cpp                                   
# )
# target_link_libraries(detsvr PRIVATE detcore)
# # target_include_directories(detsvr PUBLIC "include/")
# set_property(TARGET detsvr PROPERTY CXX_STANDARD 17)

# add_executable(videoanalysis src/videoanalysis.cpp  src/config.cpp
#                                                     src/serialization.cpp
# )
# # target_include_directories(videoanalysis PRIVATE detcore)
# target_link_libraries(videoanalysis PRIVATE detcore)
# set_property(TARGET videoanalysis PROPERTY CXX_STANDARD 17)