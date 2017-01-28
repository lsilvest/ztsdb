cmake_minimum_required(VERSION 2.8)

SET(SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../src")

ENABLE_TESTING()
FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)
INCLUDE_DIRECTORIES(${LIBCRPCUT_INCLUDE_DIRS} . ../../src)

set(SOURCE_FILES
  test.cpp
  ${SRC}/misc.cpp 
  ${SRC}/timezone/zone.cpp 
  ${SRC}/timezone/ztime.cpp 
  ${SRC}/timezone/localtime.cpp 
)

SET_SOURCE_FILES_PROPERTIES(${SRC}/timezone/localtime.cpp
  PROPERTIES COMPILE_FLAGS "-Wno-address -Wno-maybe-uninitialized")
SET_SOURCE_FILES_PROPERTIES(test.cpp
  PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations")

ADD_EXECUTABLE(test_duration ${SOURCE_FILES})
ADD_TEST(test_duration ${CMAKE_CURRENT_BINARY_DIR}/test_duration --timeout-multiplier=3)

TARGET_LINK_LIBRARIES(test_duration
  pthread 
  ${LIBCRPCUT_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  dl)
