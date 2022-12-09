
add_subdirectory(networking)

add_library(simplehttp STATIC ${ALL_OBJECT_FILES})

set(SIMPLEHTTP_LIBS
        simplehttp_networking
        )

target_link_libraries(
        simplehttp
        ${SIMPLEHTTP_LIBS})

target_include_directories(
        simplehttp PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)