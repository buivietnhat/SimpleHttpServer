add_library(
        simplehttp_api
        OBJECT
        root.cpp
       )

set(ALL_OBJECT_FILES
        ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:simplehttp_api>
        PARENT_SCOPE)