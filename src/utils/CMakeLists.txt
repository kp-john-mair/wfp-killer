
set(WFPKUTILS_SOURCES 
    utils.cpp
)

add_library(wfpkutils STATIC ${WFPKUTILS_SOURCES})

target_include_directories(wfpkutils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
