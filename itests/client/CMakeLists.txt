SET(SOURCE_FILES client.cpp)
  
ADD_EXECUTABLE(client ${SOURCE_FILES})

FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)

INCLUDE_DIRECTORIES(
  ../..
  ../../src
  ../../src/main_parser
  ${CMAKE_CURRENT_BINARY_DIR}/../../src/main_parser
  ${CMAKE_CURRENT_BINARY_DIR}/../../src
  ${LIBDOUBLE_CONVERSION_INCLUDE_DIRS})

# need to generate the parser and config header files so client.cpp can compile:


TARGET_LINK_LIBRARIES(
  client
  pthread 
  shlib
  double-conversion
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)
