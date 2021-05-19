# This makefile works on a Sandia Linux machine.
# For other platforms, you may need to adjust the "E =", "B =",
# "L =", and "W =" lines below.

# A sample Makefile building a C code using Call Scilab using Scilab binary
PATH_SCILAB = /opt/scilab-5.3.1/
# Note that PATH_SCILAB can be /usr/ is using a packaged version of Scilab.
SCILAB_CFLAGS = -I$(PATH_SCILAB)/include/scilab/ #-DDEBUG
SCILAB_LDFLAGS = -lscilab
PATH_TO_LIB_SCILAB = $(PATH_SCILAB)/lib/scilab/

LINK_OPT_1 = -Wl,-rpath,$(PATH_SCILAB)/lib/scilab -Wl,-rpath-link,$(PATH_SCILAB)/lib/scilab
LINK_OPT_2 = -Wl,-rpath,$(PATH_SCILAB)/lib/thirdparty -Wl,-rpath-link,$(PATH_SCILAB)/lib/thirdparty

# 32-bit
LINK_OPT_3 = -Wl,-rpath,$(PATH_SCILAB)/thirdparty/java/lib/i386 -Wl,-rpath-link,$(PATH_SCILAB)/thirdparty/java/lib/i386
LINK_OPT_4 = -Wl,-rpath,$(PATH_SCILAB)/thirdparty/java/lib/i386/server -Wl,-rpath-link,$(PATH_SCILAB)/thirdparty/java/lib/i386/server

# 64-bit
#LINK_OPT_3 = -Wl,-rpath,$(PATH_SCILAB)/thirdparty/java/lib/amd64 -Wl,-rpath-link,$(PATH_SCILAB)/thirdparty/java/lib/amd64
#LINK_OPT_4 = -Wl,-rpath,$(PATH_SCILAB)/thirdparty/java/lib/amd64/server -Wl,-rpath-link,$(PATH_SCILAB)/thirdparty/java/lib/amd64/server

PATH_TO_LIB_CALL_SCILAB = $(PATH_SCILAB)/lib/scilab/

dakscilab: dakscilab.c
	export LD_LIBRARY_PATH=$(PATH_TO_LIB_SCILAB):$(PATH_TO_LIB_CALL_SCILAB)
	gcc -g -o dakscilab $(LINK_OPT_1) $(LINK_OPT_2) $(LINK_OPT_3) $(LINK_OPT_4) $(SCILAB_LDFLAGS) -L$(PATH_TO_LIB_SCILAB) -L$(PATH_TO_LIB_CALL_SCILAB) $(SCILAB_CFLAGS) dakscilab.c
