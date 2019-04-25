# - Find AIO
#
# AIO_INCLUDE_DIR - Where to find libaio.h
# AIO_LIBS - List of libraries when using AIO.
# AIO_FOUND - True if AIO found.

find_path(AIO_INCLUDE_DIR
  libaio.h
  HINTS $ENV{AIO_ROOT}/include)

find_library(AIO_LIBS
  aio
  HINTS $ENV{AIO_ROOT}/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(aio DEFAULT_MSG AIO_LIBS AIO_INCLUDE_DIR)

mark_as_advanced(AIO_INCLUDE_DIR AIO_LIBS)