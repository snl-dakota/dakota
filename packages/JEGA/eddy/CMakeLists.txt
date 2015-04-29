add_subdirectory(utilities)

# JEGA includes a pthread portability interface for Windows, but the
# CMake build system doesn't yet address everything needed to build
# it, e.g., include_directories, library link, and providing config.h
if(UNIX)
  find_package(Threads)
  if(Threads_FOUND)
    add_subdirectory(threads)
  endif()
endif()
