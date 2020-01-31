# Conditionally find Java JDK if needed
macro(dakota_find_java)
  if (ENABLE_SPEC_MAINT OR ENABLE_DAKOTA_DOCS OR DAKOTA_API_JAVA)
    # Each of these requires compiling Java code with Java 1.6 or newer 
    find_package(Java 1.6 REQUIRED)
    # BMA: Not sure why Java_FOUND doesn't work here:
    if (NOT Java_JAVA_EXECUTABLE OR NOT Java_JAVAC_EXECUTABLE OR 
	NOT Java_JAR_EXECUTABLE)
      message(SEND_ERROR "Dakota spec maint, docs, and Java API require JDK.")
    endif()
    include(UseJava)
  endif()
endmacro()
