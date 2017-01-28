INCLUDE(../CMakeList.Header.txt)

set(SOURCE_FILES
  test.cpp
  ../../src/misc.cpp
)

SET_SOURCE_FILES_PROPERTIES(timezone/localtime.cpp
  PROPERTIES COMPILE_FLAGS "-Wno-address -Wno-maybe-uninitialized")
SET_SOURCE_FILES_PROPERTIES(test.cpp
  PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations")

ADD_EXECUTABLE(test_vector ${SOURCE_FILES})
ADD_TEST(test_vector ${CMAKE_CURRENT_BINARY_DIR}/test_vector --timeout-multiplier=3)

TARGET_LINK_LIBRARIES(test_vector
  pthread 
  double-conversion 
  ${LIBCRPCUT_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  dl)
