cmake_minimum_required(VERSION 2.8)

ENABLE_TESTING()
FIND_PACKAGE(LibCRPCUT REQUIRED)
FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)
INCLUDE_DIRECTORIES(${LIBCRPCUT_INCLUDE_DIRS} . ../../src)
INCLUDE_DIRECTORIES(${LIBDOUBLE_CONVERSION_INCLUDE_DIRS})

set(SOURCE_FILES
  test.cpp
  ../../src/array.hpp
  ../../src/array.cpp
  ../../src/dname.cpp
  ../../src/dname.hpp
  ../../src/misc.cpp
  ../../src/timezone/ztime.cpp
  ../../src/timezone/zone.cpp 
  ../../src/timezone/localtime.cpp)

SET_SOURCE_FILES_PROPERTIES(../../src/timezone/localtime.cpp
  PROPERTIES COMPILE_FLAGS "-Wno-address -Wno-maybe-uninitialized")

ADD_EXECUTABLE(test_array_time ${SOURCE_FILES})
ADD_TEST(test_array_time ${CMAKE_CURRENT_BINARY_DIR}/test_array_time)

TARGET_LINK_LIBRARIES(test_array_time
  pthread 
  double-conversion 
  ${LIBCRPCUT_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  dl)
