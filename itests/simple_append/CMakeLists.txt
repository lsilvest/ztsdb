FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)

SET(SRC ../../src)
SET(SRC_TZ ${SRC}/timezone)

SET(SOURCE_FILES
  simple_append.cpp
  ${SRC}/array.cpp
  ${SRC}/dname.cpp
  ${SRC}/zts.cpp
  ${SRC}/zcpp.cpp
  ${SRC}/period.cpp
  ${SRC}/misc.cpp
  ${SRC_TZ}/ztime.cpp
  ${SRC_TZ}/ztime_vector.cpp
  ${SRC_TZ}/zone.cpp
  ${SRC_TZ}/localtime.cpp
  )

SET_SOURCE_FILES_PROPERTIES(${SRC}/timezone/localtime.cpp
  PROPERTIES COMPILE_FLAGS "-Wno-address -Wno-maybe-uninitialized")

ADD_EXECUTABLE(simple_append ${SOURCE_FILES})

INCLUDE_DIRECTORIES(
  ${SRC}
  ${LIBDOUBLE_CONVERSION_INCLUDE_DIRS})

TARGET_LINK_LIBRARIES(
  simple_append
  double-conversion
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY})
