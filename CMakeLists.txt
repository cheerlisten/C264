cmake_minimum_required(VERSION 3.21)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<IF:$<CONFIG:RELEASE>,Release,Debug>)
project(C264)

option(ENABLE_TEST "enable test" TRUE)
option(ENABLE_UNITTEST "enable unit-test" TRUE)

file(GLOB C264_SOURCES "src/*.c*")
file(GLOB_RECURSE C264_HEADERS "src/inc/*.h*" )

add_library(c264 SHARED)
target_sources(c264 PRIVATE ${C264_SOURCES} ${C264_HEADERS})
target_compile_definitions(c264 PRIVATE EXPORT_C264)
target_include_directories(c264 PUBLIC src/inc PRIVATE src/inc/internal)

if(ENABLE_TEST)
    if(ENABLE_UNITTEST)
        add_subdirectory(${CMAKE_SOURCE_DIR}/extern/googletest)
    endif()
    add_subdirectory(test)
endif()