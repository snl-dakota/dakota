# CMake options for DAKOTA developer builds

# Developer options
set(DAKOTA_HAVE_MPI TRUE CACHE BOOL "Enable MPI in DAKOTA?")
set(ENABLE_DAKOTA_DOCS TRUE CACHE BOOL "Enable DAKOTA documentation build")
set(ENABLE_SPEC_MAINT TRUE CACHE BOOL 
  "Enable DAKOTA specification maintenance mode?")
set(PECOS_ENABLE_TESTS TRUE CACHE BOOL "Enable Pecos-specific tests?")

# Not included from Mike's configs, but may help some

# Disable optional X graphics
#-DHAVE_X_GRAPHICS:BOOL=FALSE
#set(HAVE_X_GRAPHICS OFF CACHE BOOL "Disable dependency on X libraries" FORCE)

# CMake 2.8.6 has problems with RHEL6/Boost -- the following is a workaround
#-DBoost_NO_BOOST_CMAKE=ON
#set(Boost_NO_BOOST_CMAKE ON CACHE BOOL "Obtain desired behavior on RHEL6" FORCE)
