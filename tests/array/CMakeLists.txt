INCLUDE(../CMakeList.Header.txt)

set(SOURCE_FILES
  test.cpp
  ../../src/array.hpp
  ../../src/array.cpp
  ../../src/dname.cpp
  ../../src/dname.hpp
  ../../src/misc.cpp
)

ADD_EXECUTABLE(test_array ${SOURCE_FILES})
ADD_TEST(test_array ${CMAKE_CURRENT_BINARY_DIR}/test_array)

TARGET_LINK_LIBRARIES(test_array 
  pthread 
  double-conversion 
  ${LIBCRPCUT_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  dl)
