#Import:
#  SPDK_INCLUDE_DIR
#  SPDK_LIBRARIES

function(import_spdk spdk_dir)
    if(NOT DPDK_DIR)
        set(DPDK_DIR "${spdk_dir}/dpdk")
    endif()

    if(NOT TARGET dpdk::dpdk)
        include(ImportDPDK)
        import_dpdk(${DPDK_DIR})
    endif()

    find_package(aio REQUIRED)
    find_package(uuid REQUIRED)

    foreach(c 
            copy_ioat
            ioat
            bdev_malloc
            bdev_null
            bdev_nvme
            nvme
            vbdev_passthru
            vbdev_error
            vbdev_gpt
            vbdev_split
            vbdev_raid
            bdev_aio
            bdev_virtio
            virtio
            sock
            sock_posix
            event_bdev
            event_copy
            blobfs
            blob
            bdev
            blob_bdev
            copy
            event
            thread
            util
            conf
            trace
            log
            lvol
            jsonrpc
            json
            rpc
            env_dpdk)
        add_library(spdk::${c} STATIC IMPORTED)
        set_target_properties(spdk::${c} PROPERTIES
            IMPORTED_LOCATION "${spdk_dir}/build/lib/${CMAKE_STATIC_LIBRARY_PREFIX}spdk_${c}${CMAKE_STATIC_LIBRARY_SUFFIX}"
            INTERFACE_INCLUDE_DIRECTORIES "${spdk_dir}/include")
        list(APPEND SPDK_LIBRARIES spdk::${c})
    endforeach()

    set_target_properties(spdk::bdev_aio PROPERTIES
        INTERFACE_LINK_LIBRARIES ${AIO_LIBS})

    set_property(TARGET spdk::env_dpdk PROPERTY
        INTERFACE_LINK_LIBRARIES 
        dpdk::dpdk numa dl rt crypto
        )

    set_target_properties(spdk::lvol PROPERTIES
        INTERFACE_LINK_LIBRARIES spdk::util)
    
    set_target_properties(spdk::util PROPERTIES
        INTERFACE_LINK_LIBRARIES ${UUID_LIBS})
    
    set(SPDK_INCLUDE_DIR "${spdk_dir}/include")

    set(SPDK_INCLUDE_DIR ${SPDK_INCLUDE_DIR} PARENT_SCOPE)
    set(SPDK_LIBRARIES ${SPDK_LIBRARIES} PARENT_SCOPE)

endfunction()

