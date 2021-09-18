add_executable(plugintest   plugintest.cpp    
                            ${PROJECT_SOURCE_DIR}/detcore/src/plugincore.cpp
)
target_include_directories(plugintest PUBLIC   ${PROJECT_SOURCE_DIR}/src
                                                ${PROJECT_SOURCE_DIR}/detcore/include
)
target_link_libraries(plugintest PRIVATE "dl")
set_property(TARGET plugintest PROPERTY CXX_STANDARD 17)
