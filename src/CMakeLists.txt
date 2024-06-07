add_subdirectory(utils)
add_subdirectory(parser)
add_subdirectory(wfpklib)
add_subdirectory(cli)

add_executable(wfpk main.cpp)

# shell32 lib is used by the IsUserAnAdmin() function
target_link_libraries(wfpk PRIVATE shell32 cli wfpklib)
