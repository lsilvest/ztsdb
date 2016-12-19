cmake_minimum_required(VERSION 2.8)

SET(SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../src")

ENABLE_TESTING()
FIND_PACKAGE(LibCRPCUT REQUIRED)
FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)
INCLUDE_DIRECTORIES(${LIBCRPCUT_INCLUDE_DIRS} . ../../src)

set(SOURCE_FILES
  test.cpp
  ../../src/misc.cpp
  ${SRC}/timezone/zone.cpp 
  ${SRC}/timezone/ztime.cpp 
  ${SRC}/timezone/localtime.cpp 
)

SET_SOURCE_FILES_PROPERTIES(../../src/timezone/localtime.cpp
  PROPERTIES COMPILE_FLAGS "-Wno-address -Wno-maybe-uninitialized")

ADD_EXECUTABLE(test_vector_set ${SOURCE_FILES})
ADD_TEST(test_vector_set ${CMAKE_CURRENT_BINARY_DIR}/test_vector_set)

TARGET_LINK_LIBRARIES(test_vector_set
  pthread 
  double-conversion 
  ${LIBCRPCUT_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  dl)
