# Include pthreads portability layer on Windows
#if(WIN32)
#  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/pthreads/include")
#endif()

add_library(ethreads
  src/condition.cpp 
  src/mutex.cpp 
  src/thread_exceptions.cpp 
  src/thread.cpp 
  src/thread_pool.cpp		
)
