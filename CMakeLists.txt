cmake_minimum_required(VERSION 2.8.12)

project(flame-sp CXX C)
set(VERSION 0.1)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
    enable_testing()
    add_subdirectory("src/googletest")
    include_directories(
        ${PROJECT_SOURCE_DIR}/src/googletest/googletest/include
        ${PROJECT_SOURCE_DIR}/src/googletest/googlemock/include)
endif(PACKAGE_TESTS)

option(SW_64 "Build for sw_64." OFF)
if(SW_64)
    set(ON_SW_64 ${SW_64})
endif(SW_64)

option(WITH_SPDK "Build with SPDK" OFF)
if(WITH_SPDK)
    set(SPDK_DIR "$ENV{SPDK_ROOT}")
    if(NOT SPDK_DIR)
        set(SPDK_DIR "${PROJECT_SOURCE_DIR}/src/spdk")
    endif()
    if(NOT EXISTS ${SPDK_DIR})
        message(FATAL_ERROR "Can't find SPDK. Please set Env Var SPDK_ROOT")
    endif()

    set(DPDK_DIR "$ENV{DPDK_ROOT}")
    if(NOT DPDK_DIR)
        set(DPDK_DIR "${SPDK_DIR}/dpdk")
    endif()
    if(NOT EXISTS ${DPDK_DIR})
        message(FATAL_ERROR "Can't find DPDK. Please set Env Var DPDK_ROOT")
    endif()

    include(ImportSPDK)
    import_spdk(${SPDK_DIR})
endif()

# you should build gperftools with './configure --enable-frame-pointers',
# as you will compile object with '-fno-omit-frame-pointer'
option(CPU_PROFILE "Build with profiler(gperftools)." OFF)

#set(CMAKE_BUILD_TYPE Debug)
# for perf 
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -O2")

option(WITH_CCACHE "Build with ccache.")
if(WITH_CCACHE)
  find_program(CCACHE_FOUND ccache)
  if(CCACHE_FOUND)
    message(STATUS "Building with ccache: ${CCACHE_FOUND}, CCACHE_DIR=$ENV{CCACHE_DIR}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    # ccache does not accelerate link (ld), but let it handle it. by passing it
    # along with cc to python's distutils, we are able to workaround
    # https://bugs.python.org/issue8027.
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
  else(CCACHE_FOUND)
    message(FATAL_ERROR "Can't find ccache. Is it installed?")
  endif(CCACHE_FOUND)
endif(WITH_CCACHE)

option(WITH_RDMA "Enable RDMA in msg module" ON)
if(WITH_RDMA)
  find_package(rdma REQUIRED)
  set(HAVE_RDMA ${RDMA_FOUND})
endif(WITH_RDMA)

#find Boost
find_package(Boost 1.46 COMPONENTS system)
#message(STATUS "${Boost_INCLUDE_DIRS}")
if(NOT Boost_FOUND)
    if(WITH_RDMA)
        message(FATAL_ERROR "Can't find boost.")
    endif(WITH_RDMA)
endif(NOT Boost_FOUND)

#find AIO
find_package(aio REQUIRED)
find_package(protobuf REQUIRED)
find_package(grpc REQUIRED)
find_package(mysql REQUIRED)
find_package(mysqlcppconn REQUIRED)

#put all the libs and binaries in one place
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIRS})

#Check Includes
include(CheckIncludeFiles)
include(CheckIncludeFileCXX)
include(CheckFunctionExists)

CHECK_INCLUDE_FILES("linux/types.h" HAVE_LINUX_TYPES_H)

include(CheckTypeSize)
set(CMAKE_EXTRA_INCLUDE_FILES "linux/types.h")
CHECK_TYPE_SIZE(__be16 __BE16) 
CHECK_TYPE_SIZE(__be32 __BE32) 
CHECK_TYPE_SIZE(__be64 __BE64) 
CHECK_TYPE_SIZE(__le16 __LE16) 
CHECK_TYPE_SIZE(__le32 __LE32) 
CHECK_TYPE_SIZE(__le64 __LE64) 
CHECK_TYPE_SIZE(__u8 __U8) 
CHECK_TYPE_SIZE(__u16 __U16) 
CHECK_TYPE_SIZE(__u32 __U32) 
CHECK_TYPE_SIZE(__u64 __U64) 
CHECK_TYPE_SIZE(__s8 __S8) 
CHECK_TYPE_SIZE(__s16 __S16) 
CHECK_TYPE_SIZE(__s32 __S32) 
CHECK_TYPE_SIZE(__s64 __S64) 
unset(CMAKE_EXTRA_INCLUDE_FILES)

include(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES("
  #include <string.h>
  int main() { char x = *strerror_r(0, &x, sizeof(x)); return 0; }
  " STRERROR_R_CHAR_P)

find_package(Threads REQUIRED)

option(ENABLE_SHARED "build shared libraries" ON)
if(ENABLE_SHARED)
    set(FLAME_SHARED SHARED)
else(ENABLE_SHARED)
    set(FLAME_SHARED STATIC)
endif(ENABLE_SHARED)
set(CMAKE_POSITION_INDEPENDENT_CODE ${ENABLE_SHARED})

option(WITH_STATIC_LIBSTDCXX "Link against libstdc++ statically" OFF)
if(WITH_STATIC_LIBSTDCXX)
  if(NOT CMAKE_COMPILER_IS_GNUCXX)
    message(FATAL_ERROR "Please use GCC to enable WITH_STATIC_LIBSTDCXX")
  endif()
endif()

set(ALLOCATOR "libc")
#if allocator is set on command line make sure it matches below strings
if(ALLOCATOR)
    if(${ALLOCATOR} MATCHES "tcmalloc(_minimal)?")
        find_package(gperftools REQUIRED)
        message(STATUS "use tcmalloc")
        set(HAVE_LIBTCMALLOC ON)
    else()
        message(STATUS "use libc malloc")
        set(ALLOCATOR "libc")
    endif()
else(ALLOCATOR)
    find_package(gperftools)
    set(HAVE_LIBTCMALLOC ${GPERFTOOLS_FOUND})
    if(GPERFTOOLS_FOUND)
        message(STATUS "use tcmalloc")
        set(ALLOCATOR tcmalloc)
    else()
        message(STATUS "use libc malloc")
        set(ALLOCATOR "libc")
    endif(GPERFTOOLS_FOUND)
endif(ALLOCATOR)


add_subdirectory(src)


