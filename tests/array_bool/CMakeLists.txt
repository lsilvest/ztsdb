INCLUDE(../CMakeList.Header.txt)

set(SOURCE_FILES
  test.cpp
  ../../src/array.hpp
  ../../src/array.cpp
  ../../src/dname.cpp
  ../../src/dname.hpp
  ../../src/misc.cpp
)

ADD_EXECUTABLE(test_array_bool ${SOURCE_FILES})
ADD_TEST(test_array_bool ${CMAKE_CURRENT_BINARY_DIR}/test_array_bool --timeout-multiplier=3)

TARGET_LINK_LIBRARIES(test_array_bool 
  pthread 
  double-conversion 
  ${LIBCRPCUT_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  dl)
