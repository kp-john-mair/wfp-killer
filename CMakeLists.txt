cmake_minimum_required(VERSION 3.15)
project(WFPKiller)

set(CMAKE_CXX_STANDARD 20)

# Add source files from the src directory
add_executable(WFPKiller
    src/wfp_killer.cpp
    src/wfp_objects.cpp
    src/wfp_name_mapper.cpp
    src/wfp_ostream_helpers.cpp)

# Link libraries
target_link_libraries(WFPKiller fwpuclnt.lib Ws2_32.lib)
