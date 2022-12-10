cmake_minimum_required(VERSION 3.10)

include(GoogleTest)

file(GLOB SIMPLEHTTP_TEST_SOURCES "${PROJECT_SOURCE_DIR}/test/*/*test.cpp")

add_custom_target(build-tests COMMAND ${CMAKE_CTEST_COMMAND} --show-only)
add_custom_target(check-tests COMMAND ${CMAKE_CTEST_COMMAND} --verbose)

foreach (simplehttp_test_source ${SIMPLEHTTP_TEST_SOURCES})
    # Create a human readable name.
    get_filename_component(simplehttp_test_filename ${simplehttp_test_source} NAME)
    string(REPLACE ".cpp" "" simplehttp_test_name ${simplehttp_test_filename})

    # Add the test target separately and as part of "make check-tests".
    add_executable(${simplehttp_test_name} EXCLUDE_FROM_ALL ${simplehttp_test_source} networking/message_parser_test.cpp)
    add_dependencies(build-tests ${simplehttp_test_name})
    add_dependencies(check-tests ${simplehttp_test_name})

    gtest_discover_tests(${simplehttp_test_name}
            EXTRA_ARGS
            --gtest_color=yes
            --gtest_output=xml:${CMAKE_BINARY_DIR}/test/${simplehttp_test_name}.xml)
    target_link_libraries(${simplehttp_test_name} simplehttp gtest gmock_main)

    # Set test target properties and dependencies.
    set_target_properties(${simplehttp_test_name}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test"
            COMMAND ${simplehttp_test_name}
            )
endforeach (simplehttp_test_source ${SIMPLEHTTP_TEST_SOURCES})