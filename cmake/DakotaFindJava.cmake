# Conditionally find Java JDK if needed
macro(dakota_find_java)
  if (ENABLE_SPEC_MAINT OR ENABLE_DAKOTA_DOCS OR DAKOTA_API_JAVA)
    set(_dakota_min_java_version 1.6)
    if (ENABLE_DAKOTA_DOCS)
      set(_dakota_min_java_version 11)
    endif()
    find_package(Java ${_dakota_min_java_version} REQUIRED)
    # BMA: Not sure why Java_FOUND doesn't work here:
    if (NOT Java_JAVA_EXECUTABLE OR NOT Java_JAVAC_EXECUTABLE OR 
	NOT Java_JAR_EXECUTABLE)
      message(SEND_ERROR "Dakota spec maint, docs, and Java API require JDK.")
    endif()
    include(UseJava)
  endif()
endmacro()
