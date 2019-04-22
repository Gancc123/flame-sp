add_executable(rdma_mgr_perf
rdma_mgr_perf.cc
get_clock.c
    )
target_link_libraries(rdma_mgr_perf common)
set_target_properties(rdma_mgr_perf
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/rdma_perf"
    )
add_custom_command(
    TARGET rdma_mgr_perf POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/flame_mgr.cfg
            ${CMAKE_BINARY_DIR}/bin/rdma_perf)

add_executable(rdma_client_perf
rdma_client_perf.cc
get_clock.c
    )
target_link_libraries(rdma_client_perf common)
set_target_properties(rdma_client_perf
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/rdma_perf"
    )

add_custom_command(
    TARGET rdma_client_perf POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/flame_client.cfg
            ${CMAKE_BINARY_DIR}/bin/rdma_perf)

file(GLOB EXTRA_FILES 
    ../*.sh 
    ../*.py
    *.cfg
    )

file(COPY ${EXTRA_FILES} DESTINATION ${CMAKE_BINARY_DIR}/bin/rdma_perf)







