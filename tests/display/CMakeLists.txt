INCLUDE(../CMakeList.Header.txt)

set(SOURCE_FILES
  test.cpp
  ../../src/dname.cpp
  ../../src/display.cpp
  ../../src/ast.cpp
  ../../src/array.cpp
  ../../src/config.cpp
  ../../src/misc.cpp
  ../../src/period.cpp
  ../../src/valuevar.cpp
  ../../src/timezone/ztime.cpp
  ../../src/timezone/zone.cpp
  ../../src/timezone/localtime.cpp
  
)

ADD_EXECUTABLE(test_display ${SOURCE_FILES})
ADD_TEST(test_display ${CMAKE_CURRENT_BINARY_DIR}/test_display)

TARGET_LINK_LIBRARIES(test_display
  pthread 
  double-conversion
  ${LIBCRPCUT_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  dl)
