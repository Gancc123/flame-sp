add_executable(rdma_mem_target
mem_cpy_target.cc
get_clock.c
    )
target_link_libraries(rdma_mem_target common)
set_target_properties(rdma_mem_target
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/rdma_mem_cpy"
    )
add_custom_command(
    TARGET rdma_mem_target POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/flame_target.cfg
            ${CMAKE_BINARY_DIR}/bin/rdma_mem_cpy)

add_executable(rdma_mem_forward
mem_cpy_forward.cc
get_clock.c
    )
target_link_libraries(rdma_mem_forward common)
set_target_properties(rdma_mem_forward
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/rdma_mem_cpy"
    )
add_custom_command(
    TARGET rdma_mem_forward POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/flame_forward.cfg
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

file(GLOB SCRIPT_FILES 
    ../*.sh 
    ../*.py
    )

file(COPY ${SCRIPT_FILES} DESTINATION ${CMAKE_BINARY_DIR}/bin/rdma_perf)


