
aux_source_directory(src UNIT_TEST_SOURCES)
add_executable(unit_test ${UNIT_TEST_SOURCES})

target_include_directories(unit_test PRIVATE src)