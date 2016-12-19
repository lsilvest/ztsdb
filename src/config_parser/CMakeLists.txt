CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(ztsdb)

IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function -Wno-sign-compare")
ELSEIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function -Wno-sign-compare -Wno-deprecated-register -Wno-unneeded-internal-declaration")
ENDIF()

FIND_PACKAGE(BISON) 
FIND_PACKAGE(FLEX) 

BISON_TARGET(cfgparser cfgparser.y
  ${CMAKE_CURRENT_SOURCE_DIR}/cfgparser.cpp
  COMPILE_FLAGS -Wno-other)

FLEX_TARGET(cfglexer cfglexer.l
${CMAKE_CURRENT_SOURCE_DIR}/cfglexer.cpp COMPILE_FLAGS
--header-file=cfglexer.hpp)

ADD_FLEX_BISON_DEPENDENCY(cfglexer cfgparser)

# define all sources here
set(SOURCE_FILES
  ${BISON_cfgparser_OUTPUTS}
  ${FLEX_cfglexer_OUTPUTS})

# create the library
ADD_LIBRARY(config_parser STATIC ${SOURCE_FILES})
