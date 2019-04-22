add_executable(msg_client
    msg_client.cc
    )
target_link_libraries(msg_client common)

add_custom_command(
    TARGET msg_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/flame_client.cfg"
            "${CMAKE_BINARY_DIR}/bin/examples/msg/")

set_target_properties(msg_client
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/examples/msg/"
    )

add_executable(msg_mgr
    msg_mgr.cc
    )
target_link_libraries(msg_mgr common)

add_custom_command(
    TARGET msg_mgr POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/flame_mgr.cfg"
            "${CMAKE_BINARY_DIR}/bin/examples/msg/")

set_target_properties(msg_mgr
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/examples/msg/"
    )






