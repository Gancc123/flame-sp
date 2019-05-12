set(OUTPUT_DIR "${TESTS_MSG_OUTPUT_DIR}/rdma_iops")

add_executable(rdma_iops_mgr
rdma_mgr.cc
get_clock.c
    )
target_link_libraries(rdma_iops_mgr common)
set_target_properties(rdma_iops_mgr
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    )
add_custom_command(
    TARGET rdma_iops_mgr POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/flame_mgr.cfg
            ${OUTPUT_DIR})

add_executable(rdma_iops_client
rdma_client.cc
get_clock.c
    )
target_link_libraries(rdma_iops_client common)
set_target_properties(rdma_iops_client
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    )
add_custom_command(
    TARGET rdma_iops_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/flame_client.cfg
            ${OUTPUT_DIR})

file(GLOB SCRIPT_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/../*.sh 
    )

file(COPY ${SCRIPT_FILES} DESTINATION ${OUTPUT_DIR})

