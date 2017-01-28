cmake_minimum_required(VERSION 2.8)

SET(SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../src")

ENABLE_TESTING()

INCLUDE_DIRECTORIES(${LIBCRPCUT_INCLUDE_DIRS} . ../../src)

set(SOURCE_FILES
  test.cpp
  ${SRC}/cow_ptr.cpp 
)

SET_SOURCE_FILES_PROPERTIES(test.cpp
  PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations")

ADD_EXECUTABLE(test_cow_ptr ${SOURCE_FILES})

ADD_TEST(test_cow_ptr ${CMAKE_CURRENT_BINARY_DIR}/test_cow_ptr --timeout-multiplier=3)

TARGET_LINK_LIBRARIES(test_cow_ptr
  pthread 
  ${LIBCRPCUT_LIBRARIES}
  dl)
