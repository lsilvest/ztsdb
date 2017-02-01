CMAKE_MINIMUM_REQUIRED(VERSION 2.8.8)
PROJECT(ztsdb)

ENABLE_TESTING()
FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)
FIND_PACKAGE(LibDOUBLE_CONVERSION REQUIRED)

INCLUDE_DIRECTORIES(
  ${LIBDOUBLE_CONVERSION_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/main_parser
  ${CMAKE_CURRENT_SOURCE_DIR}/main_parser) # for location.hpp

SET(SOURCE_FILES
  juice/mpl.hpp
  juice/variant.hpp
  juice/variant_binary.hpp
  timezone/localtime.cpp
  timezone/private.h
  timezone/tz.hpp
  timezone/tzfile.h
  timezone/zone.cpp
  timezone/zone.hpp
  timezone/ztime.cpp
  timezone/ztime.hpp
  timezone/ztime_vector.cpp
  timezone/ztime_vector.hpp
  timezone/interval.hpp
  timezone/vector_set_time.hpp
  align_funcs.hpp
  allocator.hpp
  anf.cpp
  anf.hpp
  api.hpp
  array.cpp
  array.hpp
  array_ops.hpp
  ast.cpp
  ast.hpp
  base_funcs_array.cpp
  base_funcs_array_idx.cpp
  base_funcs.cpp
  base_funcs.hpp
  base_funcs_ic.cpp
  base_funcs_math.cpp
  base_funcs_set.cpp
  base_funcs_roll.cpp
  base_types.cpp
  base_types.hpp
  ${CMAKE_CURRENT_BINARY_DIR}/cmdline.h
  ${CMAKE_CURRENT_BINARY_DIR}/cmdline.c
  config.cpp
  config_ctx.cpp
  config_ctx.hpp
  config.hpp
  conversion_funcs.cpp
  conversion_funcs.hpp
  cow_ptr.cpp
  cow_ptr.hpp
  csv.cpp
  csv.hpp
  display.cpp
  display.hpp
  dname.cpp
  dname.hpp
  double.hpp
  encode.cpp
  encode.hpp
  env.hpp
  globals.hpp
  index.hpp
  info.hpp
  interp.cpp
  interp_ctx.cpp
  interp_ctx.hpp
  interp_error.cpp
  interp_error.hpp
  interp.hpp
  load_builtin.cpp
  load_builtin.hpp
  logging.hpp
  misc.cpp
  misc.hpp
  msg_handler.cpp
  msg_handler.hpp
  net_client.cpp
  net_client.hpp
  net_handler.cpp
  net_handler.hpp
  parser_ctx.cpp
  parser_ctx.hpp
  parser_utils.hpp
  period.cpp
  period.hpp
  pseudoarray.hpp
  pseudovector.hpp
  stats.hpp
  string.cpp
  string.hpp
  type_utils.hpp
  unop_binop_funcs.cpp
  unop_binop_funcs.hpp
  valuevar.cpp
  valuevar.hpp
  valuevar_ic.cpp
  valuevar_ic.hpp
  valuevector.hpp
  vector_base.hpp
  vector.hpp
  vector_set.hpp
  zcpp.cpp
  zcpp.hpp
  zcpp_stdlib.hpp   
  zts.cpp
  zts.hpp
  )


EXECUTE_PROCESS(COMMAND
  "git" describe --abbrev=6 --dirty --always --tags
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_VERSION
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

FIND_PROGRAM(GENGETOPT gengetopt)
IF (NOT GENGETOPT) 
   MESSAGE(FATAL_ERROR "gengetopt executable not found")
ENDIF()


ADD_CUSTOM_COMMAND(OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/cmdline.c   ${CMAKE_CURRENT_BINARY_DIR}/cmdline.h
                   COMMAND ${GENGETOPT} --set-version=${GIT_VERSION} -i ${CMAKE_CURRENT_SOURCE_DIR}/cmdline.ggo --output-dir=${CMAKE_CURRENT_BINARY_DIR}/ --long-help --show-required
                   DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/cmdline.ggo  ${GENGETOPTDEP}
                   VERBATIM)

SET_SOURCE_FILES_PROPERTIES(timezone/localtime.cpp
  PROPERTIES COMPILE_FLAGS "-Wno-address -Wno-maybe-uninitialized")

ADD_SUBDIRECTORY(main_parser)
ADD_SUBDIRECTORY(config_parser)

ADD_EXECUTABLE(ztsdb
  ztsdb.cpp
  $<TARGET_OBJECTS:ztsdb_lib>
  $<TARGET_OBJECTS:main_parser>
  $<TARGET_OBJECTS:config_parser>)

ADD_LIBRARY(ztsdb_lib OBJECT ${SOURCE_FILES})
ADD_DEPENDENCIES(ztsdb_lib main_parser config_parser)

TARGET_LINK_LIBRARIES(
  ztsdb
  pthread 
  dl
  double-conversion 
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  )


SET(HEADER_FILES
  main_parser/location.hpp
  main_parser/position.hpp
  ast.hpp
  anf.hpp
  display.hpp
  globals.hpp
  timezone/interval.hpp
  timezone/zone.hpp
  timezone/tz.hpp
  timezone/tzfile.h
  timezone/ztime_vector.hpp
  timezone/ztime.hpp
  cow_ptr.hpp
  string.hpp
  dname.hpp
  array.hpp
  juice/variant.hpp
  juice/mpl.hpp
  allocator_factory.hpp
  allocator.hpp
  misc.hpp
  vector.hpp
  vector_base.hpp
  period.hpp
  type_utils.hpp
  base_types.hpp
  logging.hpp
  net_client.hpp
  encode.hpp
  valuevar.hpp
  juice/variant_binary.hpp
  juice/variant.hpp
  zts.hpp
  valuevar_ic.hpp
  valuevector.hpp
  net_handler.hpp
  stats.hpp
  info.hpp
  config.hpp
  index.hpp
)

INSTALL(TARGETS ztsdb DESTINATION bin)
INSTALL(FILES ${HEADER_FILES} DESTINATION include/ztsdb)
