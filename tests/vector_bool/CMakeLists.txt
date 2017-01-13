INCLUDE(../CMakeList.Header.txt)

set(SOURCE_FILES
  test.cpp
  ../../src/misc.cpp
)

ADD_EXECUTABLE(test_vector_bool ${SOURCE_FILES})
ADD_TEST(test_vector_bool ${CMAKE_CURRENT_BINARY_DIR}/test_vector_bool --timeout-multiplier=3)

TARGET_LINK_LIBRARIES(test_vector_bool
  pthread 
  double-conversion 
  ${LIBCRPCUT_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  dl)
