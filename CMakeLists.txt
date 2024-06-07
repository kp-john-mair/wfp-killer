cmake_minimum_required(VERSION 3.15)
project(WFPKiller)
set(CMAKE_CXX_STANDARD 20)

# Enable 'ctest' usage (to run all our tests, as specified in tests/CMakeLists.txt)
enable_testing()

set(WFP_KILLER_VENDOR_DIR ${PROJECT_SOURCE_DIR}/vendor)

add_subdirectory(src)
add_subdirectory(tests)
