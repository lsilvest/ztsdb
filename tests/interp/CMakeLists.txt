INCLUDE(../CMakeList.Header.txt)

set(SOURCE_FILES test.cpp)

ADD_EXECUTABLE(test_interp $<TARGET_OBJECTS:ztsdb_lib> ${SOURCE_FILES})
ADD_TEST(test_interp ${CMAKE_CURRENT_BINARY_DIR}/test_interp --timeout-multiplier=3)

TARGET_LINK_LIBRARIES(test_interp
  pthread 
  double-conversion
  ${OBJECT_FILES}
  ${LIBCRPCUT_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  dl
  main_parser
  config_parser)
