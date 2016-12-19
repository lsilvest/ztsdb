cmake_minimum_required(VERSION 2.8)

set(zscript "${CMAKE_CURRENT_BINARY_DIR}/../src/ztsdb")

file(GLOB runit_files
     "runit*.R")

add_custom_target(rtest)
foreach(f ${runit_files})
  add_custom_command(TARGET rtest
                     COMMAND ${zscript} -e "'source(\"${CMAKE_CURRENT_SOURCE_DIR}/RUnitMini.R\");dummy<-runTestFile(\"${f}\")'"
                     WORKING_DIRECTORY "../../Rtests")
endforeach()
