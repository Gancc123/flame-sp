# Try to find libuuid
# Once done, this will define
#
# UUID_FOUND - system has Profiler
# UUID_INCLUDE_DIR - the Profiler include directories
# UUID_LIBS - link these to use Profiler

if(UUID_INCLUDE_DIR AND UUID_LIBS)
  set(UUID_FIND_QUIETLY TRUE)
endif()

find_path(UUID_INCLUDE_DIR NAMES uuid/uuid.h)
find_library(UUID_LIBS NAMES uuid)
set(UUID_LIBS ${LIBUUID})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(uuid DEFAULT_MSG UUID_LIBS UUID_INCLUDE_DIR)

mark_as_advanced(UUID_LIBS UUID_INCLUDE_DIR)