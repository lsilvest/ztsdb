cmake_minimum_required(VERSION 2.8)

SET(SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../src")

ENABLE_TESTING()

FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)
FIND_PACKAGE(LibDOUBLE_CONVERSION REQUIRED)

INCLUDE_DIRECTORIES(
  ${LIBCRPCUT_INCLUDE_DIRS}
  ../../src
  ../../src/main_parser
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/../../src/main_parser
  ${LIBDOUBLE_CONVERSION_INCLUDE_DIRS})

set(SOURCE_FILES
  test.cpp
  ${SRC}/anf.cpp 
  ${SRC}/array.cpp 
  ${SRC}/ast.cpp 
  ${SRC}/dname.cpp 
  ${SRC}/parser_ctx.cpp 
  ${SRC}/misc.cpp
  ${SRC}/timezone/ztime.cpp
  ${SRC}/timezone/zone.cpp 
  ${SRC}/timezone/localtime.cpp
)

SET_SOURCE_FILES_PROPERTIES(../../src/timezone/localtime.cpp
  PROPERTIES COMPILE_FLAGS "-Wno-address -Wno-maybe-uninitialized")
SET_SOURCE_FILES_PROPERTIES(test.cpp
  PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations")

ADD_EXECUTABLE(test_anf $<TARGET_OBJECTS:main_parser> ${SOURCE_FILES})
ADD_TEST(test_anf ${CMAKE_CURRENT_BINARY_DIR}/test_anf --timeout-multiplier=3)

TARGET_LINK_LIBRARIES(test_anf
  pthread 
  double-conversion 
  ${LIBCRPCUT_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  dl)
