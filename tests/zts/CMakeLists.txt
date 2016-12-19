INCLUDE(../CMakeList.Header.txt)

SET(SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../src")

set(SOURCE_FILES
  test.cpp
  ${SRC}/zts.cpp
  ${SRC}/dname.cpp
  ${SRC}/display.cpp
  ${SRC}/config.cpp
  ${SRC}/ast.cpp
  ${SRC}/array.cpp
  ${SRC}/valuevar.cpp
  ${SRC}/period.cpp
  ${SRC}/misc.cpp
  ${SRC}/timezone/zone.cpp 
  ${SRC}/timezone/ztime.cpp 
  ${SRC}/timezone/localtime.cpp 
)

ADD_EXECUTABLE(test_zts ${SOURCE_FILES})
ADD_TEST(test_zts ${CMAKE_CURRENT_BINARY_DIR}/test_zts)

TARGET_LINK_LIBRARIES(test_zts
  pthread 
  double-conversion 
  ${LIBCRPCUT_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  dl)
