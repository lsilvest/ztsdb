FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)

SET(BASE ../..)
SET(SRC ${BASE}/src)

SET(SOURCE_FILES
  simple_append.cpp
  ${SRC}/zcpp.cpp
  ${SRC}/misc.cpp
  )

SET_SOURCE_FILES_PROPERTIES(${SRC}/timezone/localtime.cpp
  PROPERTIES COMPILE_FLAGS "-Wno-address -Wno-maybe-uninitialized")

ADD_EXECUTABLE(simple_append ${SOURCE_FILES})

INCLUDE_DIRECTORIES(
  ${BASE}
  ${LIBDOUBLE_CONVERSION_INCLUDE_DIRS})

TARGET_LINK_LIBRARIES(
  simple_append
  double-conversion
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY})
