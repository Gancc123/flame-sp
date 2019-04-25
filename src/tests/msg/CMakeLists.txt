set(TESTS_MSG_OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/tests/msg)

add_subdirectory(test_msg)
add_subdirectory(test_rdma)
add_subdirectory(rdma_perf)
add_subdirectory(test_mem_cpy)
add_subdirectory(rdma_iops)

package_add_test(buddy_allocator_unittest buddy_allocator_unittest.cc)

target_link_libraries(buddy_allocator_unittest common)

set_target_properties(buddy_allocator_unittest
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${TESTS_MSG_OUTPUT_DIR}
    )