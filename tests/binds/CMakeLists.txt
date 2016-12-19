INCLUDE(../CMakeList.Header.txt)

set(SOURCE_FILES test.cpp)

ADD_EXECUTABLE(test_binds $<TARGET_OBJECTS:ztsdb_lib> ${SOURCE_FILES})
ADD_TEST(test_binds ${CMAKE_CURRENT_BINARY_DIR}/test_binds)

TARGET_LINK_LIBRARIES(test_binds
  pthread 
  double-conversion
  ${OBJECT_FILES}
  ${LIBCRPCUT_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  dl
  main_parser
  config_parser)
