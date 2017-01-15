CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(ztsdb)

FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)
FIND_PACKAGE(LibDOUBLE_CONVERSION REQUIRED)
FIND_PACKAGE(BISON)
FIND_PACKAGE(FLEX)

SET(MIN_BISON 3.0.2)
IF (BISON_VERSION VERSION_LESS MIN_BISON)
  MESSAGE(FATAL_ERROR "BISON version must be at least ${MIN_BISON}!")
ENDIF()

SET(SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)
SET(SRC_TZ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src/timezone)
SET(SRC_PRSR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src/main_parser)

BISON_TARGET(parser
  ${SRC_PRSR_DIR}/parser.y
  ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
  VERBOSE dummy.txt
  )

FLEX_TARGET(lexer
  ${SRC_PRSR_DIR}/lexer.l
  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp
  COMPILE_FLAGS "--header-file=${CMAKE_CURRENT_BINARY_DIR}/lexer.hpp")
ADD_FLEX_BISON_DEPENDENCY(lexer parser)

INCLUDE_DIRECTORIES(${LIBDOUBLE_CONVERSION_INCLUDE_DIRS}
  ${SRC_PRSR_DIR}
  ${CMAKE_CURRENT_BINARY_DIR})              

SET(SOURCE_FILES
  ${SRCDIR}/parser_ctx.cpp
  ${SRCDIR}/anf.cpp
  ${SRCDIR}/array.cpp
  ${SRCDIR}/ast.cpp
  ${SRCDIR}/valuevar.cpp
  ${SRCDIR}/dname.cpp
  ${SRCDIR}/net_handler.cpp
  ${SRCDIR}/encode.cpp
  ${SRCDIR}/misc.cpp
  ${SRCDIR}/zts.cpp
  ${SRCDIR}/display.cpp
  ${SRCDIR}/conversion_funcs.cpp
  ${SRCDIR}/string.cpp
  ${SRCDIR}/base_types.cpp
  ${SRCDIR}/unop_binop_funcs.cpp
  ${SRCDIR}/config.cpp
  ${SRCDIR}/zcpp.cpp
  ${SRCDIR}/period.cpp
  ${SRCDIR}/net_client.cpp
  ${BISON_parser_OUTPUTS}
  ${FLEX_lexer_OUTPUTS})

SET(TZ_SOURCE_FILES
  ${SRC_TZ_DIR}/ztime.cpp
  ${SRC_TZ_DIR}/ztime_vector.cpp
  ${SRC_TZ_DIR}/zone.cpp
  ${SRC_TZ_DIR}/localtime.cpp)

SET_SOURCE_FILES_PROPERTIES(${SRC_TZ_DIR}/localtime.cpp
  PROPERTIES COMPILE_FLAGS "-Wno-address -Wno-maybe-uninitialized")

SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp
  PROPERTIES COMPILE_FLAGS "-Wno-unused-function")

# create the shared library; -fPIC is the default which is what we
# want (besides winning the lottery)
ADD_LIBRARY(shlib SHARED
  ${SOURCE_FILES}
  ${TZ_SOURCE_FILES}
  ${MAIN_PARSER_SOURCE_FILES}
  )

SET_TARGET_PROPERTIES(shlib PROPERTIES OUTPUT_NAME ztsdb)

INSTALL(TARGETS shlib DESTINATION lib)
