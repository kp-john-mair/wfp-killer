file(GLOB_RECURSE SOURCES "*.cpp")

# Print the gathered sources for verification
message(STATUS "Library Sources: ${SOURCES}")

# Add the library
add_library(wfpklib STATIC ${SOURCES})

# Add the include directory
target_include_directories(wfpklib PUBLIC .)
target_include_directories(wfpklib PUBLIC ../../vendor/cxxopts/include)

