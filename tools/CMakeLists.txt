add_executable(keygen keygen.cpp)
target_link_libraries(keygen PRIVATE detcore)
set_property(TARGET keygen PROPERTY CXX_STANDARD 17)