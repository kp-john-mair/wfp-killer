file(GLOB_RECURSE SOURCES "*.cpp")

# Print the gathered sources for verification
message(STATUS "Library Sources: ${SOURCES}")

# Add the library
add_library(wfpklib STATIC ${SOURCES})

# Ensure all our MS libs are linked (this replaces the #pragma lib directives)
target_link_libraries(wfpklib
    PUBLIC
    ws2_32
    fwpuclnt
    shell32
    rpcrt4
)

# Add the include directory
target_include_directories(wfpklib PUBLIC .)
target_include_directories(wfpklib PUBLIC ../../vendor/cxxopts/include)
target_include_directories(wfpklib PUBLIC ../../vendor/magic_enum/include)
