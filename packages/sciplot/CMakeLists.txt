cmake_minimum_required(VERSION 2.8.9)
project(DakotaSciPlot)

find_package(X11 REQUIRED)
find_package(Motif REQUIRED)

# Set the export name for install targets; parent packages likely want
# to override this to the name of their target
set(ExportTarget ${CMAKE_PROJECT_NAME}-targets CACHE STRING 
  "Name for the export target for ${CMAKE_PROJECT_NAME}")

include_directories(${X11_INCLUDE_DIR} ${MOTIF_INCLUDE_DIR})

# We only need the HAVE_XPM definition, unless the defines need to
# propagate to the system libraries (hope not).
#remove_definitions("-DHAVE_CONFIG_H")

if(X11_Xpm_FOUND)
  add_definitions("-DHAVE_XPM")
endif(X11_Xpm_FOUND)

add_library(dakota_sciplot
  Board.c 
  choosecol.c 
  Common.c 
  Frame.c 
  icon.c 
  long.c 
  Pen.c 
  RowCol.c 
  SciPlot.c 
  scroll.c 
  str2color.c 
  strarray.c 
  StrToPmap.c 
  ThWheel.c 
  XCC.c 
  )

#EXTRA_DIST = darkstipple.bm \
#	     lightstipple.bm \
#	     wheel0.pm \
#	     wheel1.pm \
#	     wheel2.pm \
#	     wheel3.pm \
#	     wheel0h.pm \
#	     wheel1h.pm \
#	     wheel2h.pm \
#	     wheel3h.pm \
#	     SciPlotDocs/SciPlotDemo.gif \
#	     SciPlotDocs/SciPlotExample.gif \
#	     SciPlotDocs/SciPlotTitle.gif \
#	     SciPlotDocs/new.gif \
#	     SciPlotDocs/SciPlot.html \
#	     SciPlotDocs/SciPlotDemo.html \
#	     SciPlotDocs/SciPlotProg.html \
#	     XPM/diamond0.pm \
#	     XPM/diamond1.pm \
#	     XPM/ERROR.pm \
#	     XPM/FATAL.pm \
#	     XPM/INFO.pm \
#	     XPM/QUESTION.pm \
#	     XPM/square0.pm \
#	     XPM/square1.pm \
#	     XPM/WARNING.pm

# TODO: Install package configuration and import files (.cmake files, targets).

install(TARGETS dakota_sciplot EXPORT ${ExportTarget} DESTINATION lib)
