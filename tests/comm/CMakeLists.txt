INCLUDE(../CMakeList.Header.txt)

set(SOURCE_FILES test.cpp)

ADD_EXECUTABLE(test_comm
  $<TARGET_OBJECTS:ztsdb_lib>
  $<TARGET_OBJECTS:main_parser>
  $<TARGET_OBJECTS:config_parser>
  ${SOURCE_FILES})

ADD_TEST(test_comm ${CMAKE_CURRENT_BINARY_DIR}/test_comm --timeout-multiplier=3)

TARGET_LINK_LIBRARIES(test_comm
  pthread 
  double-conversion
  ${OBJECT_FILES}
  ${LIBCRPCUT_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  dl)
