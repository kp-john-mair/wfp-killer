
set(PARSER_SOURCES 
    lexer.cpp
    parser.cpp
)

# Add the library
add_library(parser STATIC ${PARSER_SOURCES})

# Ensure all our MS libs are linked (this replaces the #pragma lib directives)
target_link_libraries(parser
    PRIVATE
    ws2_32
    fwpuclnt
    rpcrt4
)

target_link_libraries(parser PUBLIC wfpkutils)

# Add the include directory
target_include_directories(parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(parser PRIVATE ${WFP_KILLER_VENDOR_DIR}/magic_enum/include)
