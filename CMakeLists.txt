cmake_minimum_required(VERSION 3.16)
project(SimpleHttpServer)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

# Includes.
set(SIMPLEHTTP_SRC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/include)
set(SIMPLEHTTP_TEST_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/test/include)

include_directories(${SIMPLEHTTP_SRC_INCLUDE_DIR} ${SIMPLEHTTP_TEST_INCLUDE_DIR})
include_directories(BEFORE src) # This is needed for gtest.

enable_testing()

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(third_party)

add_dependencies(gtest simplehttp)

set(MAIN_SOURCES main.cpp)
add_executable(main ${MAIN_SOURCES})

target_link_libraries(main simplehttp Threads::Threads)

