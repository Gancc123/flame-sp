add_executable(rdma_client
    rdma_client.cc
    )
target_link_libraries(rdma_client common)

add_custom_command(
    TARGET rdma_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/flame_client.cfg
            ${CMAKE_BINARY_DIR}/bin/rdma)

set_target_properties(rdma_client
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/rdma"
    )

add_executable(rdma_mgr
    rdma_mgr.cc
    )
target_link_libraries(rdma_mgr common)

add_custom_command(
    TARGET rdma_mgr POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/flame_mgr.cfg
            ${CMAKE_BINARY_DIR}/bin/rdma)

set_target_properties(rdma_mgr
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/rdma"
    )


package_add_test(rdma_buffer_unittest rdma_buffer_unittest.cc)
target_link_libraries(rdma_buffer_unittest common)
set_target_properties(rdma_buffer_unittest
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/rdma"
    )


add_executable(rdma_rw_mgr
    rdma_rw_mgr.cc
    )
target_link_libraries(rdma_rw_mgr common)
set_target_properties(rdma_rw_mgr
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/rdma"
    )

add_executable(rdma_rw_client
    rdma_rw_client.cc
    )
target_link_libraries(rdma_rw_client common)
set_target_properties(rdma_rw_client
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/rdma"
    )





