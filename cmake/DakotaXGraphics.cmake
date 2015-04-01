# Try to find X Windows headers and libraries needed for Dakota X
# graphics capability (X11, Motif, pthread).  If found, set
# DAKOTA_X_DEPS_FOUND
macro(dakota_x_graphics)

  # Survey of headers called out in src/ and packages/motif/
  
  # Directly included headers:
  #   X11/Core.h X11/xpm.h Xm/Xm.h)
  # Secondarily included headers:
  #   X11/Composite.h X11/Core.h X11/CoreP.h X11/Intrinsic.h
  #   X11/IntrinsicP.h X11/Shell.h X11/ShellP.h X11/StringDefs.h
  #   X11/Xatom.h X11/Xlib.h X11/Xlibint.h X11/Xmu/CharSet.h
  #   X11/Xmu/Converters.h X11/Xmu/Drawing.h X11/Xutil.h X11/xpm.h
  #   Xm/CascadeB.h Xm/DialogS.h Xm/Form.h Xm/Frame.h Xm/Label.h
  #   Xm/MainW.h Xm/PushB.h Xm/RowColumn.h Xm/ToggleB.h Xm/Xm.h)
  # Critical to check a Motif header, since not in X11
  # Not checking for now: sys/param.h unistd.h pthread.h

  # Rough library dep map from RHEL6:
  # X11: Xau
  # Xmu: Xt, Xext, X11, SM, ICE, Xau, xcb
  # Xpm: X11, xcb, Xau
  # Xm: Xmu, Xt, Xext, X11, Xp, Xft, jpeg, png12, SM, ICE, xcb, Xau, 
  #     fontconfig, freetype, Xrender
  #
  # May also need (Mac):
  # Xdmcp
  #
  # Historically when statically linking, we've needed the following:
  # Xt, Xext, Xau

  set(DAKOTA_X_DEPS_FOUND FALSE)
  find_package(X11)
  find_package(Motif)
  find_package(Threads)

  if (# X11_FOUND is TRUE if X11_X11_INCLUDE_PATH AND X11_LIBRARIES:
      X11_FOUND AND 
      # MOTIF_FOUND is TRUE if header and library found
      MOTIF_FOUND AND 
      CMAKE_USE_PTHREADS_INIT AND
      # Specifically require headers explicitly mentioned in Dakota
      # src and packages/motif, but not libraries, in case they are
      # bundled or differently named.  May need to tighten this
      # requirement.  All these will be in X11_INCLUDE_DIR.
      X11_Xmu_INCLUDE_PATH   AND   # Xmu/*.h
      X11_Xpm_INCLUDE_PATH   AND   # xpm.h
      X11_Xt_INCLUDE_PATH    AND   # Intrinsic.h
      X11_Xlib_INCLUDE_PATH  AND   # Xlib.h
      X11_Xutil_INCLUDE_PATH       # Xutil.h
      )
  
      set(DAKOTA_X_DEPS_FOUND TRUE)
      
      set(DAKOTA_X_INCLUDE_DIR ${MOTIF_INCLUDE_DIR} ${X11_INCLUDE_DIR})
      list(REMOVE_DUPLICATES DAKOTA_X_INCLUDE_DIR)

      # Previously Dakota called out X11, Motif, Xpm, Xmu, Xt, threads
      # X11, Xext, SM, ICE should already be in X11_LIBRARIES...
      set(DAKOTA_X_LIBRARIES ${MOTIF_LIBRARIES})
      foreach(xlib Xmu Xt Xext Xpm Xrender X11 Xft SM ICE Xau Xdmcp)
	if(X11_${xlib}_LIB)
	  list(APPEND DAKOTA_X_LIBRARIES ${X11_${xlib}_LIB})
	endif()
      endforeach()
      list(APPEND DAKOTA_X_LIBRARIES ${X11_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

  endif()

endmacro()