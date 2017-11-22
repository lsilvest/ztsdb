CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(ztsdb)


## from https://cmake.org/pipermail/cmake/2009-June/030311.html
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)
## end from


## modified from: http://stackoverflow.com/a/18369825:
##
SET(MIN_GCC 4.9)
SET(MIN_CLANG 3.6)

IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  IF (CMAKE_CXX_COMPILER_VERSION VERSION_LESS MIN_GCC)
    MESSAGE(FATAL_ERROR "GCC version must be at least ${MIN_GCC}!")
  ENDIF()
ELSEIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  IF (CMAKE_CXX_COMPILER_VERSION VERSION_LESS MIN_CLANG)
    MESSAGE(FATAL_ERROR "Clang version must be at least ${MIN_CLANG}!")
  ENDIF()
ELSE()
  MESSAGE(WARNING "You are using an unsupported compiler! Compilation"
    "has only been tested with Clang and GCC.")
ENDIF()
## end modified from

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")


ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(shlib)
ADD_SUBDIRECTORY(shlib_client)
ADD_SUBDIRECTORY(itests/client)
ADD_SUBDIRECTORY(itests/append)
ADD_SUBDIRECTORY(itests/simple_append)
ADD_SUBDIRECTORY(itests/simple_append_c)


ENABLE_TESTING()

FIND_PACKAGE(LibCRPCUT QUIET)

if (LIBCRPCUT_FOUND)
  ADD_SUBDIRECTORY(tests/cow_ptr)
  ADD_SUBDIRECTORY(tests/anf)
  ADD_SUBDIRECTORY(tests/ast)
  ADD_SUBDIRECTORY(tests/vector)
  ADD_SUBDIRECTORY(tests/vector_bool)
  ADD_SUBDIRECTORY(tests/vector_set)
  ADD_SUBDIRECTORY(tests/zstring)
  ADD_SUBDIRECTORY(tests/time)
  ADD_SUBDIRECTORY(tests/duration)
  ADD_SUBDIRECTORY(tests/period)
  ADD_SUBDIRECTORY(tests/array)
  ADD_SUBDIRECTORY(tests/array_time)
  ADD_SUBDIRECTORY(tests/array_bool)
  ADD_SUBDIRECTORY(tests/parser)
  ADD_SUBDIRECTORY(tests/interp)
  ADD_SUBDIRECTORY(tests/interp_time)
  ADD_SUBDIRECTORY(tests/interp_error)
  ADD_SUBDIRECTORY(tests/encode)
  ADD_SUBDIRECTORY(tests/zts)
  ADD_SUBDIRECTORY(tests/mmap)
  ADD_SUBDIRECTORY(tests/display)
  ADD_SUBDIRECTORY(tests/csv)
  ADD_SUBDIRECTORY(tests/control)
  ADD_SUBDIRECTORY(tests/config)
  ADD_SUBDIRECTORY(tests/comm)
  ADD_SUBDIRECTORY(tests/comm_append)
  ADD_SUBDIRECTORY(tests/binds)
  ADD_SUBDIRECTORY(tests/align)
elseif (NOT LIBCRPCUT_FOUND)
  MESSAGE(WARNING "crpcut not found, utests will not be generated")
endif (LIBCRPCUT_FOUND)

ADD_SUBDIRECTORY(Rtests)
