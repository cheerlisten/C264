
link_libraries(c264)
aux_source_directory(singles singles)
include_directories(include)
add_compile_definitions(CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\")

link_libraries(GTest::gtest_main)

foreach(single ${singles})
    string(REGEX REPLACE "singles\/([^.]*)\.(.*)" "\\1" single_name ${single})
    add_executable(${single_name} ${single})
    message("add single:${single_name} src=${single}")
endforeach()

if(ENABLE_UNITTEST)
    add_subdirectory(unit_test)
endif()
