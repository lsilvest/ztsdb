INCLUDE(../CMakeList.Header.txt)

set(SOURCE_FILES
  test.cpp
  ../../src/array.cpp
  ../../src/dname.cpp
  ../../src/misc.cpp
  ../../src/period.cpp
  ../../src/zts.cpp
  ../../src/timezone/ztime.cpp
  ../../src/timezone/zone.cpp 
  ../../src/timezone/localtime.cpp)

ADD_EXECUTABLE(test_align ${SOURCE_FILES})
ADD_TEST(test_align ${CMAKE_CURRENT_BINARY_DIR}/test_align --timeout-multiplier=3)

TARGET_LINK_LIBRARIES(test_align
  pthread 
  double-conversion 
  ${LIBCRPCUT_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  dl)
