set(OUTPUT_DIR "${TESTS_MSG_OUTPUT_DIR}/test_spdk")

add_executable(rdma_mgr_perf_spdk
    rdma_mgr_perf.cc
    get_clock.c
    )
target_link_libraries(rdma_mgr_perf_spdk 
    common
    ${CMAKE_DL_LIBS}
    ${SPDK_LIBRARIES}
    )
set_target_properties(rdma_mgr_perf_spdk
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    )
add_custom_command(
    TARGET rdma_mgr_perf_spdk POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/flame_mgr.cfg
            ${OUTPUT_DIR})

add_executable(rdma_client_perf_spdk
    rdma_client_perf.cc
    get_clock.c
    )
target_link_libraries(rdma_client_perf_spdk
    common
    ${CMAKE_DL_LIBS}
    ${SPDK_LIBRARIES}
    )
set_target_properties(rdma_client_perf_spdk
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    )

add_custom_command(
    TARGET rdma_client_perf_spdk POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/flame_client.cfg
            ${OUTPUT_DIR})

file(GLOB EXTRA_FILES 
    ../*.sh 
    ../*.py
    *.cfg
    )

file(COPY ${EXTRA_FILES} DESTINATION ${OUTPUT_DIR})







