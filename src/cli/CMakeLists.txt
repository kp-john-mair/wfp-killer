
set(CLI_SOURCES 
    create_command.cpp
    delete_command.cpp
    list_command.cpp
    monitor_command.cpp
)

# Add the library
add_library(cli STATIC ${CLI_SOURCES})

target_link_libraries(cli PUBLIC wfpklib)

# Add the include directory
target_include_directories(cli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(cli PUBLIC ${WFP_KILLER_VENDOR_DIR}/cxxopts/include)
