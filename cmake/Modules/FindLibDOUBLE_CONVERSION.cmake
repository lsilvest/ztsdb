# - Try to find libdouble-conversion
# Once done this will define
#
#  LIBDOUBLE_CONVERSION_FOUND - system has libcrpcut
#  LIBDOUBLE_CONVERSION_INCLUDE_DIRS - the libcrpcut include directory
#  LIBDOUBLE_CONVERSION_LIBRARIES - Link these to use libcrpcut
#  LIBDOUBLE_CONVERSION_DEFINITIONS - Compiler switches required for using libcrpcut
#
#  Copyright (c) 2008 Bernhard Walle <bernhard.walle@gmx.de>
#  Copyright (c) 2016 Leonardo Silvestri
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (LIBDOUBLE_CONVERSION_LIBRARIES AND LIBDOUBLE_CONVERSION_INCLUDE_DIRS)
  set (LibDOUBLE_CONVERSION_FIND_QUIETLY TRUE)
endif (LIBDOUBLE_CONVERSION_LIBRARIES AND LIBDOUBLE_CONVERSION_INCLUDE_DIRS)

find_path (LIBDOUBLE_CONVERSION_INCLUDE_DIRS
    NAMES
      double-conversion.h
    PATHS
      /usr/include
      /usr/include/double-conversion
      /usr/local/include
      /usr/local/include/double-conversion
      /opt/local/include
      /opt/local/include/double-conversion
      ENV CPATH)

find_library (LIBDOUBLE_CONVERSION_LIBRARIES
    NAMES
      double-conversion
    PATHS
      /usr/lib
      /usr/lib64
      /usr/local/lib
      /usr/local/lib64
      /opt/local/lib
      /opt/usr/lib64
      ENV LIBRARY_PATH
      ENV LD_LIBRARY_PATH)

include (FindPackageHandleStandardArgs)


# handle the QUIETLY and REQUIRED arguments and set LIBDOUBLE_CONVERSION_FOUND to TRUE if all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibDOUBLE_CONVERSION DEFAULT_MSG
    LIBDOUBLE_CONVERSION_LIBRARIES
    LIBDOUBLE_CONVERSION_INCLUDE_DIRS)


mark_as_advanced(LIBDOUBLE_CONVERSION_INCLUDE_DIRS LIBDOUBLE_CONVERSION_LIBRARIES)
