add_executable(keygen keygen.cpp)
target_link_libraries(keygen PRIVATE detcore)
set_property(TARGET keygen PROPERTY CXX_STANDARD 17)

add_executable(simplesvr simplesvr.cpp  ../src/service.cpp
                                        ../src/config.cpp
                                        ../src/serialization.cpp
)
target_link_libraries(simplesvr PRIVATE detcore)
set_property(TARGET simplesvr PROPERTY CXX_STANDARD 17)

add_executable(genconfig    genconfig.cpp
                            ../src/config.cpp
                            ../src/serialization.cpp
)
target_link_libraries(genconfig PRIVATE detcore)
set_property(TARGET genconfig PROPERTY CXX_STANDARD 17)

add_executable(videoplayer  videoplayer.cpp 
                            ../src/config.cpp
                            ../src/serialization.cpp
)
target_link_libraries(videoplayer PRIVATE detcore)
set_property(TARGET videoplayer PROPERTY CXX_STANDARD 17)

add_executable(loopdetect  loopdetect.cpp 
                            ../src/config.cpp
                            ../src/serialization.cpp
)
target_link_libraries(loopdetect PRIVATE detcore)
set_property(TARGET loopdetect PROPERTY CXX_STANDARD 17)