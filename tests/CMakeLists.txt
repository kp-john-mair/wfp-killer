include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.12.0
)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

# Function to add a test
# function(add_gtest test_name test_source)
#     add_executable(${test_name} ${test_source})
#     target_link_libraries(${test_name} PRIVATE GTest::GTest wfpklib)
#     add_test(NAME ${test_name} COMMAND ${test_name})
# endfunction()

# # Find all test files in the tests directory
# file(GLOB TEST_SOURCES "*.cpp")

# # Add each test file
# foreach(TEST_SOURCE ${TEST_SOURCES})
#     # Get the file name without the directory
#     get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
#     # Add the test
#     add_gtest(${TEST_NAME} ${TEST_SOURCE})
# endforeach()

add_executable(lexer_test lexer_test.cpp)
target_link_libraries(lexer_test PRIVATE GTest::GTest wfpklib)
add_test(lexer_gtests lexer_test)

add_executable(parser_test parser_test.cpp)
target_link_libraries(parser_test PRIVATE GTest::GTest wfpklib)
add_test(parser_gtests parser_test)


