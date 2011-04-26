cmake_minimum_required(VERSION 2.8)
project(DakotaMotif)

find_package(X11 REQUIRED)
find_package(Motif REQUIRED)

include_directories(include src/Widgets 
                    ${X11_INCLUDE_DIR} 
		    ${MOTIF_INCLUDE_DIR}
)

# We only need the HAVE_XPM definition, unless the defines need to
# propagate to the system libraries (hope not).
remove_definitions("-DHAVE_CONFIG_H")

if(X11_Xpm_FOUND)
  add_definitions("-DHAVE_XPM")
endif(X11_Xpm_FOUND)

add_library(DGraphics
                          src/Graph2D.C 
			  src/Graphics.C 
			  src/menu.c 
			  src/Widgets/Board.c 
			  src/Widgets/choosecol.c 
			  src/Widgets/Common.c 
			  src/Widgets/Frame.c 
			  src/Widgets/icon.c 
			  src/Widgets/long.c 
			  src/Widgets/Pen.c 
			  src/Widgets/RowCol.c 
			  src/Widgets/SciPlot.c 
			  src/Widgets/scroll.c 
			  src/Widgets/str2color.c 
			  src/Widgets/strarray.c 
			  src/Widgets/StrToPmap.c 
			  src/Widgets/ThWheel.c 
			  src/Widgets/ToggleBplus.c 
			  src/Widgets/XCC.c 
)


#EXTRA_DIST = Widgets/darkstipple.bm \
#	     Widgets/lightstipple.bm \
#	     Widgets/wheel0.pm \
#	     Widgets/wheel1.pm \
#	     Widgets/wheel2.pm \
#	     Widgets/wheel3.pm \
#	     Widgets/wheel0h.pm \
#	     Widgets/wheel1h.pm \
#	     Widgets/wheel2h.pm \
#	     Widgets/wheel3h.pm \
#	     Widgets/SciPlotDocs/SciPlotDemo.gif \
#	     Widgets/SciPlotDocs/SciPlotExample.gif \
#	     Widgets/SciPlotDocs/SciPlotTitle.gif \
#	     Widgets/SciPlotDocs/new.gif \
#	     Widgets/SciPlotDocs/SciPlot.html \
#	     Widgets/SciPlotDocs/SciPlotDemo.html \
#	     Widgets/SciPlotDocs/SciPlotProg.html \
#	     Widgets/XPM/diamond0.pm \
#	     Widgets/XPM/diamond1.pm \
#	     Widgets/XPM/ERROR.pm \
#	     Widgets/XPM/FATAL.pm \
#	     Widgets/XPM/INFO.pm \
#	     Widgets/XPM/QUESTION.pm \
#	     Widgets/XPM/square0.pm \
#	     Widgets/XPM/square1.pm \
#	     Widgets/XPM/WARNING.pm


# Install headers.
install(
  FILES include/Graph2D.H include/Graphics.H include/menu.h
  DESTINATION include
  )

# Install binaries.
#install(
#  TARGETS fsudace ${fsu_executables}
#  EXPORT  FSUDace-targets
#  ARCHIVE DESTINATION lib
#  LIBRARY DESTINATION lib
#  RUNTIME DESTINATION bin
#  )

# Install package configuration and import files.
##configure_file(${FSUDace_SOURCE_DIR}/cmake/FSUDaceConfig.cmake.in
##               ${FSUDace_BINARY_DIR}/cmake/FSUDaceConfig.cmake @ONLY)
##install(FILES ${FSUDace_BINARY_DIR}/cmake/FSUDaceConfig.cmake
##       DESTINATION lib/cmake/FSUDace)
##install(EXPORT FSUDace-targets DESTINATION lib/cmake/FSUDace)
