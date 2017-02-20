FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)

SET(BASE ../..)
SET(SRC ${BASE}/src)

SET(SOURCE_FILES
  simple_append.c
  ${SRC}/zcpp.cpp
  ${SRC}/misc.cpp
  )

ADD_EXECUTABLE(simple_append_c ${SOURCE_FILES})

INCLUDE_DIRECTORIES(
  ${BASE}
  ${LIBDOUBLE_CONVERSION_INCLUDE_DIRS})

TARGET_LINK_LIBRARIES(
  simple_append_c
  double-conversion
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY})
