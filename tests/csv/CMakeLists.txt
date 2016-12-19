INCLUDE(../CMakeList.Header.txt)

SET(SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../src")

set(SOURCE_FILES
  test.cpp
  ${SRC}/base_types.cpp
  ${SRC}/config.cpp
  ${SRC}/csv.cpp
  ${SRC}/dname.cpp
  ${SRC}/display.cpp
  ${SRC}/ast.cpp
  ${SRC}/array.cpp
  ${SRC}/misc.cpp
  ${SRC}/zts.cpp
  ${SRC}/period.cpp
  ${SRC}/valuevar.cpp
  ${SRC}/timezone/zone.cpp 
  ${SRC}/timezone/ztime.cpp 
  ${SRC}/timezone/ztime_vector.cpp 
  ${SRC}/timezone/localtime.cpp 
)

ADD_EXECUTABLE(test_csv ${SOURCE_FILES})
ADD_TEST(test_csv ${CMAKE_CURRENT_BINARY_DIR}/test_csv)

TARGET_LINK_LIBRARIES(test_csv
  pthread 
  double-conversion 
  ${LIBCRPCUT_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  dl)
