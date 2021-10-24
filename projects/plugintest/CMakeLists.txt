cmake_minimum_required(VERSION 3.10)
project(plugintest LANGUAGES C CXX VERSION 1.0)

add_subdirectory(minic)

add_executable(plugintest plugintest.cpp)
# target_include_directories(plugintest PRIVATE   
#                            ${PROJECT_SOURCE_DIR}/../detcore/include)
target_link_libraries(plugintest PRIVATE detcore)
set_property(TARGET plugintest PROPERTY CXX_STANDARD 17)
