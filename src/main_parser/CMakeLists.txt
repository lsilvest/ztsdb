CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(ztsdb)

IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function -Wno-sign-compare")
ELSEIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function -Wno-sign-compare -Wno-deprecated-register -Wno-unneeded-internal-declaration")
ENDIF()

FIND_PACKAGE(BISON)
FIND_PACKAGE(FLEX)

SET(MIN_BISON 3.0.2)
IF (BISON_VERSION VERSION_LESS MIN_BISON)
  MESSAGE(FATAL_ERROR "BISON version must be at least ${MIN_BISON}!")
ENDIF()

BISON_TARGET(parser
  parser.y
  ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)

FLEX_TARGET(lexer
  lexer.l
  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp
  COMPILE_FLAGS --header-file=${CMAKE_CURRENT_BINARY_DIR}/lexer.hpp)
ADD_FLEX_BISON_DEPENDENCY(lexer parser)

# define all sources here
set(SOURCE_FILES
  ${BISON_parser_OUTPUTS}
  ${FLEX_lexer_OUTPUTS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/main_parser)

# create the library
ADD_LIBRARY(main_parser STATIC ${SOURCE_FILES})
