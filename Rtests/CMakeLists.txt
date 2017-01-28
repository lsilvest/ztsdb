cmake_minimum_required(VERSION 2.8)

set(zscript "${CMAKE_CURRENT_BINARY_DIR}/../src/ztsdb")


add_custom_target(rtest)

file(GLOB r_files "*.R")
foreach(f ${r_files})
  configure_file(${f} "${CMAKE_CURRENT_BINARY_DIR}/${fn}" COPYONLY)
endforeach()

file(GLOB runit_files "runit*.R")
foreach(f ${runit_files})
  add_custom_command(TARGET rtest
                     COMMAND ${zscript} -e "'source(\"${CMAKE_CURRENT_BINARY_DIR}/RUnitMini.R\");q(runTestFile(\"${f}\"))'"
                     WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
endforeach()
