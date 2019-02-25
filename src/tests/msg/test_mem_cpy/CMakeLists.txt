add_executable(rdma_mem_mgr
rdma_mgr.cc
get_clock.c
    )
target_link_libraries(rdma_mem_mgr common)
set_target_properties(rdma_mem_mgr
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/rdma_mem_cpy"
    )
add_custom_command(
    TARGET rdma_mem_mgr POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/flame_mgr.cfg
            ${CMAKE_BINARY_DIR}/bin/rdma_mem_cpy)

add_executable(rdma_mem_client
rdma_client.cc
get_clock.c
    )
target_link_libraries(rdma_mem_client common)
set_target_properties(rdma_mem_client
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/rdma_mem_cpy"
    )
add_custom_command(
    TARGET rdma_mem_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/flame_client.cfg
            ${CMAKE_BINARY_DIR}/bin/rdma_mem_cpy)

add_executable(local_mem_cpy
local_mem_cpy.cc
get_clock.c
    )
target_link_libraries(local_mem_cpy common)
set_target_properties(local_mem_cpy
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/rdma_mem_cpy"
    )





