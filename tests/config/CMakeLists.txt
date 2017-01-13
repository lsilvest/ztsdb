INCLUDE(../CMakeList.Header.txt)

SET(SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../src")

set(SOURCE_FILES
  test.cpp
  ${SRC}/config.cpp
  ${SRC}/config_ctx.cpp
  ${SRC}/array.cpp
  ${SRC}/dname.cpp
  ${SRC}/misc.cpp
  ${SRC}/timezone/zone.cpp 
  ${SRC}/timezone/ztime.cpp 
  ${SRC}/timezone/localtime.cpp 
)

ADD_EXECUTABLE(test_config ${SOURCE_FILES})
ADD_TEST(test_config ${CMAKE_CURRENT_BINARY_DIR}/test_config --timeout-multiplier=3)

TARGET_LINK_LIBRARIES(test_config
  pthread 
  double-conversion 
  ${LIBCRPCUT_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  dl
  main_parser
  config_parser)

