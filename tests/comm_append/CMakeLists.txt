INCLUDE(../CMakeList.Header.txt)

set(SOURCE_FILES test.cpp)

ADD_EXECUTABLE(test_comm_append $<TARGET_OBJECTS:ztsdb_lib> ${SOURCE_FILES})
ADD_TEST(test_comm_append ${CMAKE_CURRENT_BINARY_DIR}/test_comm_append)

TARGET_LINK_LIBRARIES(test_comm_append
  pthread 
  double-conversion
  ${OBJECT_FILES}
  ${LIBCRPCUT_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  dl
  main_parser
  config_parser)
