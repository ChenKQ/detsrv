find_package(OpenCV)
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(PkgConfig REQUIRED)
# pkg_check_modules(depglib REQUIRED IMPORTED_TARGET glib-2.0)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
# target_link_libraries(target PkgConfig::deps)

add_executable(videoanalysis videoanalysis.cpp 
                             io.cpp
                            ../plugincore.cpp 
                            ../config.cpp
                            ../utils.cpp
)
target_include_directories(videoanalysis PRIVATE ${OpenCV_INCLUDE_DIRS} 
                                                "../../include"
                                                ${GLIB_INCLUDE_DIRS}
                                                ${GSTREAMER_INCLUDE_DIRS}
)
target_link_libraries(videoanalysis PRIVATE ${OpenCV_LIBS} "dl" "pthread"
                                            ${GLIB_LIBRARIES}
                                            ${GSTREAMER_LIBRARIES}
                                            "gstrtspserver-1.0"

)