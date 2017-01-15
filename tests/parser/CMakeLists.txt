cmake_minimum_required(VERSION 2.8)

SET(SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../src")

ENABLE_TESTING()
FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)
INCLUDE_DIRECTORIES(
  ${LIBCRPCUT_INCLUDE_DIRS}
  ../../src
  ../../src/main_parser
  ${CMAKE_CURRENT_BINARY_DIR}/../../src/main_parser
  ${CMAKE_CURRENT_BINARY_DIR}
  ${LIBDOUBLE_CONVERSION_INCLUDE_DIRS})

set(SOURCE_FILES
  test.cpp
  ${SRC}/array.cpp 
  ${SRC}/dname.cpp 
  ${SRC}/ast.cpp 
  ${SRC}/anf.cpp 
  ${SRC}/misc.cpp 
  ${SRC}/parser_ctx.cpp 
  ${SRC}/timezone/zone.cpp 
  ${SRC}/timezone/ztime.cpp 
  ${SRC}/timezone/localtime.cpp 
)

SET_SOURCE_FILES_PROPERTIES(../../src/timezone/localtime.cpp
  PROPERTIES COMPILE_FLAGS "-Wno-address -Wno-maybe-uninitialized")

ADD_EXECUTABLE(test_parser
  $<TARGET_OBJECTS:main_parser> 
  ${SOURCE_FILES})

ADD_TEST(test_parser ${CMAKE_CURRENT_BINARY_DIR}/test_parser --timeout-multiplier=3)

TARGET_LINK_LIBRARIES(test_parser 
  pthread 
  double-conversion
  ${LIBCRPCUT_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  dl)
