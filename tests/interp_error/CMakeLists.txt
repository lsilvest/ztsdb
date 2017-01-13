INCLUDE(../CMakeList.Header.txt)

set(SOURCE_FILES
  test.cpp
  ../../src/interp_error.cpp
)

ADD_EXECUTABLE(test_interp_error ${SOURCE_FILES})
ADD_TEST(test_interp_error ${CMAKE_CURRENT_BINARY_DIR}/test_interp_error --timeout-multiplier=3)

TARGET_LINK_LIBRARIES(test_interp_error
  pthread 
  ${LIBCRPCUT_LIBRARIES}
  dl)
