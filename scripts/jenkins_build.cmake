# Setup the build with desirable build settings from a jenkins ExecuteShell step

set(CTEST_SITE "$ENV{NODE_NAME}")
set(CTEST_DASHBOARD_ROOT "$ENV{WORKSPACE}")
set(CTEST_SOURCE_DIRECTORY "${CTEST_DASHBOARD_ROOT}/source")
set(CTEST_BINARY_DIRECTORY "${CTEST_DASHBOARD_ROOT}/build")

set(CTEST_BUILD_NAME "$ENV{JOB_NAME}")

set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
set(CTEST_BUILD_CONFIGURATION RelWithDebInfo)
set(CTEST_BUILD_COMMAND "make -i -j8" )

set(CTEST_CONFIGURE_COMMAND
  "${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE:STRING=${CTEST_BUILD_CONFIGURATION}")
set(CTEST_CONFIGURE_COMMAND
  "${CTEST_CONFIGURE_COMMAND} \"-G${CTEST_CMAKE_GENERATOR}\"")
set(CTEST_CONFIGURE_COMMAND
  "${CTEST_CONFIGURE_COMMAND} \"-C${CTEST_DASHBOARD_ROOT}/BuildSetup.cmake\"")
set(CTEST_CONFIGURE_COMMAND
  "${CTEST_CONFIGURE_COMMAND} \"-C${CTEST_SOURCE_DIRECTORY}/config/DakotaDistro.cmake\"")
set(CTEST_CONFIGURE_COMMAND
  "${CTEST_CONFIGURE_COMMAND} \"${CTEST_SOURCE_DIRECTORY}\"")


# Now execute the "configure/make/make install" steps

ctest_start(Experimental)

ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}")
ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}")

# ToDo:  Verify "install" step is combined with build when using ctest

# ToDo:  Confirm the submit step is the only thing giving BMA heartburn
####ctest_submit()

