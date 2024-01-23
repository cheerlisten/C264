cmake_minimum_required(VERSION 3.21)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<IF:$<CONFIG:RELEASE>,Release,Debug>)
set(CMAKE_CXX_STANDARD 17)
project(C264)

option(ENABLE_TEST "enable test" TRUE)
option(ENABLE_UNITTEST "enable unit-test" TRUE)

file(GLOB_RECURSE C264_INTERNAL_HEADERS "src/inc/*.h")
file(GLOB C264_INTERFACE_HEADERS "src/inc/c264/*.h")

foreach(interface_header ${C264_INTERFACE_HEADERS})
    list(REMOVE_ITEM C264_INTERNAL_HEADERS ${interface_header})
endforeach()

file(GLOB_RECURSE C264_SOURCES "src/*.c*")

add_library(c264 SHARED)
target_sources(c264 PRIVATE
    ${C264_SOURCES}
    ${C264_INTERNAL_HEADERS}
    ${C264_INTERFACE_HEADERS})
target_compile_definitions(c264 PRIVATE EXPORT_C264)
target_include_directories(c264 PUBLIC src/inc PRIVATE src/inc/internal)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/src/inc/c264 PREFIX Public FILES ${C264_INTERFACE_HEADERS})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/src/inc/internal PREFIX Private/Headers FILES ${C264_INTERNAL_HEADERS})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/src PREFIX Private/Sources FILES ${C264_SOURCES} ${C264_SYNTAX_SOURCES})

if(ENABLE_TEST)
    if(ENABLE_UNITTEST)
        add_subdirectory(${CMAKE_SOURCE_DIR}/extern/googletest)
    endif()

    add_subdirectory(test)
endif()