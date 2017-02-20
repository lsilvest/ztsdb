FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)

SET(BASE ../..)
SET(SRC ${BASE}/src)

SET(SOURCE_FILES
  append.cpp
  ${SRC}/zcpp.cpp
  ${SRC}/misc.cpp
  )

ADD_EXECUTABLE(append ${SOURCE_FILES})

INCLUDE_DIRECTORIES(
  ${BASE}
  ${LIBDOUBLE_CONVERSION_INCLUDE_DIRS})

TARGET_LINK_LIBRARIES(
  append
  double-conversion
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY})
