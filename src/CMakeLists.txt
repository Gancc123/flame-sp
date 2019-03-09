set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format-security -Wno-return-type")

# requeired by spdlog
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")

if(CPU_PROFILE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} \
-Wl,--no-as-needed -lprofiler -Wl,--as-needed" )
endif(CPU_PROFILE)

# sort out which allocator to use
if(ALLOCATOR STREQUAL "tcmalloc")
    set(ALLOC_LIBS ${GPERFTOOLS_TCMALLOC_LIBRARY})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin-malloc \
-fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free")
    set(TCMALLOC_srcs perfglue/heap_profiler.cc)
elseif(ALLOCATOR STREQUAL "tcmalloc_minimal")
    set(ALLOC_LIBS ${GPERFTOOLS_TCMALLOC_MINIMAL_LIBRARY})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin-malloc \
-fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free")
    set(TCMALLOC_srcs perfglue/disabled_heap_profiler.cc)
elseif(ALLOCATOR STREQUAL "libc")
    set(TCMALLOC_srcs perfglue/disabled_heap_profiler.cc)
else()
    set(TCMALLOC_srcs perfglue/disabled_heap_profiler.cc)
    message(FATAL_ERROR "Unsupported allocator selected: ${ALLOCATOR}")
endif()

if(HAVE_RDMA)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${RDMA_INCLUDE_DIR}")
  list(APPEND EXTRALIBS ${RDMA_LIBRARIES})
endif(HAVE_RDMA)

include(TestBigEndian)
test_big_endian(FLAME_BIG_ENDIAN)
if(NOT FLAME_BIG_ENDIAN)
    set(FLAME_LITTLE_ENDIAN 1)
endif()

if(LINUX)
    add_definitions("-D_GNU_SOURCE")
endif()

# Now create a useable config.h
configure_file(
    ${CMAKE_SOURCE_DIR}/src/include/acconfig-h.in.cmake
    ${CMAKE_BINARY_DIR}/include/acconfig.h
)

set(flame_common_deps
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    )
if(HAVE_RDMA)
    list(APPEND flame_common_deps ${RDMA_LIBRARY})
endif(HAVE_RDMA)

set(msg_common_srcs
    msg/internal/errno.cc
    msg/internal/node_addr.cc
    msg/internal/msg_config.cc
    msg/internal/types_helper.cc
    msg/internal/util.cc
    msg/NetHandler.cc
    msg/event/EventPoller.cc
    msg/Msg.cc
    msg/MsgWorker.cc
    msg/Session.cc
    msg/MsgManager.cc
    msg/socket/TcpListenPort.cc
    msg/socket/TcpConnection.cc
    msg/socket/TcpStack.cc
    msg/dispatcher/MsgDispatcher.cc)

set(msg_rdma_common_srcs)
if(HAVE_RDMA)
  list(APPEND msg_rdma_common_srcs
    msg/rdma/BuddyAllocator.cc
    msg/rdma/Infiniband.cc
    msg/rdma/RdmaMem.cc
    msg/rdma/MemoryManager.cc
    msg/rdma/RdmaListenPort.cc
    msg/rdma/RdmaPrepConn.cc
    msg/rdma/RdmaConnection.cc
    msg/rdma/RdmaStack.cc)
endif(HAVE_RDMA)

set(libcommon_files
    common/thread/io_priority.cc
    common/thread/signal.cc
    common/thread/mutex.cc
    common/thread/thread.cc
    common/log.cc
    common/config.cc
    common/context.cc
    util/fs_util.cc
    util/str_util.cc
    util/option_parser.cc
    ${msg_common_srcs}
    ${msg_rdma_common_srcs}
    msg/Stack.cc
    msg/msg_context.cc
    )


add_library(common-objs OBJECT ${libcommon_files})

set(flame_common_objs
    $<TARGET_OBJECTS:common-objs>)

add_library(common STATIC ${flame_common_objs})
target_link_libraries(common
  PRIVATE ${flame_common_deps} ${ALLOC_LIBS})
if(WITH_STATIC_LIBSTDCXX)
    # the apps linking against libcommon are daemons also written in C++, so we
    # need to link them against libstdc++.
    target_link_libraries(common
        INTERFACE "-static-libstdc++ -static-libgcc")
endif()


add_library(flame-common SHARED ${flame_common_objs})
target_link_libraries(flame-common ${flame_common_deps} ${ALLOC_LIBS})
# appease dpkg-shlibdeps
set_target_properties(flame-common PROPERTIES
    SOVERSION 0
    INSTALL_RPATH "")
if(WITH_STATIC_LIBSTDCXX)
    # link libstdc++ into flame-common to avoid including libstdc++ in every 
    # apps, to reduce the size of the app linking against flame-common.
    set_target_properties(flame-common PROPERTIES
      LINK_FLAGS "-static-libstdc++ -static-libgcc")
endif()

add_subdirectory(examples)
add_subdirectory(tests)
