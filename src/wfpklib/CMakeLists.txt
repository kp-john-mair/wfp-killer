
set(WFPKLIB_SOURCES 
    wfp_killer.cpp
    wfp_name_mapper.cpp
    wfp_objects.cpp
    wfp_ostream_helpers.cpp
)

add_library(wfpklib STATIC ${WFPKLIB_SOURCES})

target_link_libraries(wfpklib PRIVATE parser)
target_link_libraries(wfpklib PUBLIC wfpkutils)

target_include_directories(wfpklib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
