set(OUTPUT_DIR "${LIBCHUNK_DIR}")

# libchunk test

#### libchunk
add_library(libchunk-objs OBJECT
    libchunk.cc
    cmd_service_mapper.cc
    msg_recv.cc
    )
list(APPEND obj_modules libchunk)


#### gen xxx_objs
foreach(module IN LISTS obj_modules)
    # generate *.pb.h first of all
    # add_dependencies(${module}-objs x_pb_tgt)
    set(${module}_objs $<TARGET_OBJECTS:${module}-objs>)
endforeach()


add_executable(server_test 
    ${libchunk_objs}
    server_test.cc
    )

# add_library(libchunk STATIC ${libchunk_objs})
target_link_libraries(server_test common)
set_target_properties(server_test
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    )

add_executable(client_test
    ${libchunk_objs}
    client_test.cc
    )
target_link_libraries(client_test common)
set_target_properties(client_test
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    )

