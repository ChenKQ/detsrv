cmake_minimum_required(VERSION 3.10)
project(detcore LANGUAGES C CXX)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -Wl,-rpath=./")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wl,-rpath=./")

find_package(OpenCV)
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)

add_library(detcore SHARED src/base64.cpp 
                    src/io.cpp
                    src/plugincore.cpp
                    src/utils.cpp
)
target_include_directories(detcore PUBLIC   ${PROJECT_SOURCE_DIR}/include
                                            ${OpenCV_INCLUDE_DIRS}
                                            ${GLIB_INCLUDE_DIRS}
                                            ${GSTREAMER_INCLUDE_DIRS}
)
target_link_libraries(detcore PUBLIC    "dl" "pthread" "gstrtspserver-1.0"
                                        ${OpenCV_LIBS}
                                        ${GLIB_LIBRARIES}
                                        ${GSTREAMER_LIBRARIES}
)
set_property(TARGET detcore PROPERTY CXX_STANDARD 17)