cmake_minimum_required(VERSION 2.8.4)
project(amplsolver)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_MODULE_PATH})

set(ExportTarget ${CMAKE_PROJECT_NAME}-targets CACHE STRING
  "Name for the export target for ${CMAKE_PROJECT_NAME}")

set(AMPL_INCLUDE_DIRS ${amplsolver_SOURCE_DIR}
                      ${amplsolver_BINARY_DIR} PARENT_SCOPE)
                      
include_directories(${amplsolver_SOURCE_DIR}
                    ${amplsolver_BINARY_DIR})

set(genarith_srcs arithchk.c)
if(WIN32)
  set_source_files_properties(arithchk.c PROPERTIES COMPILE_FLAGS "-DNO_FPINIT")
else()
  list(APPEND genarith_srcs fpinit.c)
endif()
add_executable(genarith ${genarith_srcs})
if(UNIX)
  target_link_libraries(genarith m)
endif()


configure_file(${amplsolver_SOURCE_DIR}/stdio1.h0 
  ${amplsolver_BINARY_DIR}/stdio1.h)

if(UNIX)
  find_program(UNAME uname)
  if(UNAME)
    execute_process(COMMAND ${UNAME} -sr OUTPUT_VARIABLE System_details)
  endif()
else()
  set(System_details "${CMAKE_SYSTEM_NAME}")
endif()
string(REPLACE "\n" ""  System_details "${System_details}")
configure_file(${amplsolver_SOURCE_DIR}/details.c0.cmake.in
  ${amplsolver_BINARY_DIR}/details.c)

# If genarith command fails, a zero length arith.h will be generated,
# so remove on failure to cause later build failure
add_custom_command(
  OUTPUT ${amplsolver_BINARY_DIR}/arith.h
  COMMAND genarith > ${amplsolver_BINARY_DIR}/arith.h || 
    ${CMAKE_COMMAND} -E remove ${amplsolver_BINARY_DIR}/arith.h
) 

set(amplsolver_SRCS
  ${amplsolver_BINARY_DIR}/arith.h
  ${amplsolver_BINARY_DIR}/details.c
  asldate.c
  atof.c
  auxinfo.c
  avldelete.c
  avltree.c
  b_search.c
  basename.c
  bscanf.c
  com2eval.c
  comeval.c
  con1ival.c
  con2ival.c
  con2val.c
  conadj.c
  conpval.c
  conscale.c
  conval.c
  derprop.c
  dtoa1.c
  duthes.c
  dynlink.c
  f_read.c
  fg_read.c
  fg_write.c
  fgh_read.c
  fpecatch.c
  fpinit.c
  fullhes.c
  func_add.c
  funcadd1.c
  g_fmt.c
  genrowno.c
  getenv.c
  getstub.c
  htcl.c
  jac0dim.c
  jac2dim.c
  jacdim.c
  jacinc.c
  jacinc1.c
  mach.c
  mainexit.c
  mip_pri.c
  misc.c
  mypow.c
  names.c
  nl_obj.c
  nqpcheck.c
  obj2val.c
  obj_prec.c
  objconst.c
  objval.c
  objval_.c
  op_type.c
  pfg_read.c
  pfghread.c
  printf.c
  pshvprod.c
  punknown.c
  qp_read.c
  qpcheck.c
  qsortv.c
  readsol.c
  repwhere.c
  rops.c
  rops2.c
  sigcatch.c
  sos_add.c
  sphes.c
  sscanf.c
  stderr.c
  studchk0.c
  suf_sos.c
  value.c
  writesol.c
  wrtsol_.c
  ws_desc.c
  wsu_desc.c
  x2check.c
  xectim.c
  xp1known.c
  xp2known.c)

add_library(amplsolver ${amplsolver_SRCS})

option(AMPL_HAVE_DLOPEN "Toggle support for dlopen in AMPL" OFF)
if (AMPL_HAVE_DLOPEN)
  find_package(DL)
  if (DL_FOUND)
    add_definitions("-DAMPL_HAVE_DLOPEN")
  else()
    message(WARNING "dlopen requested, but not found")
  endif()
endif()

if(UNIX)
  target_link_libraries(amplsolver ${DL_LIBRARY})
endif()

# Allow AMPL build to be shared among several source packages
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/AMPLConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/AMPLConfig.cmake
)

install(TARGETS amplsolver EXPORT ${ExportTarget} DESTINATION lib)
#install(TARGETS genarith EXPORT ${ExportTarget} DESTINATION bin)

file(GLOB ampl_headers *.h *.hd)
install(FILES ${ampl_headers} DESTINATION include/ampl)
install(FILES ${amplsolver_BINARY_DIR}/arith.h ${amplsolver_BINARY_DIR}/stdio1.h
  DESTINATION include/ampl)

