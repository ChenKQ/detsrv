add_executable(simplesvr simplesvr.cpp  ../src/utils.cpp
                                        ../src/service.cpp
                                        ../src/config.cpp
                                        )
target_link_libraries(simplesvr "pthread")
target_include_directories(simplesvr PRIVATE "../include")
set_property(TARGET simplesvr PROPERTY CXX_STANDARD 17)
set_target_properties(simplesvr PROPERTIES 
                        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)

add_executable(genconfig   genconfig.cpp
                            ../src/config.cpp
                            ../src/utils.cpp
)
target_include_directories(genconfig PRIVATE "../include")
set_target_properties(genconfig PROPERTIES 
                        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)