cmake_minimum_required(VERSION 3.15)
project(WFPKiller)

set(CMAKE_CXX_STANDARD 20)

# Automatically add all .cpp files in the src directory
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Add source files to the executable
add_executable(WFPKiller ${SOURCES})

# Include the header-only library directory for the option library
target_include_directories(WFPKiller PRIVATE vendor/cxxopts/include)

# Include directory for our project headers
target_include_directories(WFPKiller PUBLIC src)
