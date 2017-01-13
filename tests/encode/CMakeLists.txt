INCLUDE(../CMakeList.Header.txt)

set(SOURCE_FILES test.cpp)

ADD_EXECUTABLE(test_encode $<TARGET_OBJECTS:ztsdb_lib> ${SOURCE_FILES})
ADD_TEST(test_encode ${CMAKE_CURRENT_BINARY_DIR}/test_encode --timeout-multiplier=10)

SET_SOURCE_FILES_PROPERTIES(test.cpp PROPERTIES COMPILE_FLAGS "-Wno-strict-overflow")

TARGET_LINK_LIBRARIES(test_encode
  pthread 
  double-conversion
  ${OBJECT_FILES}
  ${LIBCRPCUT_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  dl
  main_parser
  config_parser)
