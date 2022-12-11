add_library(
        simplehttp_networking
        OBJECT
        socket.cpp
        http_server.cpp
        connection_manager.cpp
        message_parser.cpp
        http_message_define.cpp)

set(ALL_OBJECT_FILES
        ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:simplehttp_networking>
        PARENT_SCOPE)