cmake_minimum_required(VERSION 3.15)
project(WFPKiller)
set(CMAKE_CXX_STANDARD 20)

# Enable 'ctest' usage (to run all our tests, as specified in tests/CMakeLists.txt)
enable_testing()

add_subdirectory(src)
add_subdirectory(tests)
