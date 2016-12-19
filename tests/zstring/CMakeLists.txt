INCLUDE(../CMakeList.Header.txt)

set(SOURCE_FILES
  test.cpp
  ../../src/string.cpp
)

ADD_EXECUTABLE(test_zstring ${SOURCE_FILES})
ADD_TEST(test_zstring ${CMAKE_CURRENT_BINARY_DIR}/test_zstring)

TARGET_LINK_LIBRARIES(test_zstring
  pthread 
  double-conversion 
  ${LIBCRPCUT_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  dl)
