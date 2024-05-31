# Include the library directory
add_subdirectory(library)

# Add source files to the executable
add_executable(wfpk main.cpp)

# Include directory for our project headers
# shell32 lib is used by the IsUserAnAdmin() function
target_link_libraries(wfpk PRIVATE shell32 wfpklib)
