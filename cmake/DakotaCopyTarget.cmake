# Macro: DakotaCopyTarget
# Copy the executable given by target_name to the current binary dir
# and add to provided list of dependencies
macro(DakotaCopyTarget target_name deplist_varname)
  set(executable_name ${target_name}${CMAKE_EXECUTABLE_SUFFIX})
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${executable_name}
    COMMAND ${CMAKE_COMMAND} 
      -E copy $<TARGET_FILE:${target_name}> ${CMAKE_CURRENT_BINARY_DIR}/
    # Since we pass it a target name, DEPENDS will create a file-level 
    # dependency so this command gets run whenever the executable is rebuilt.
    DEPENDS ${target_name}
    COMMENT "Copying ${executable_name} to ${CMAKE_CURRENT_BINARY_DIR}/"
  )
  list(APPEND ${deplist_varname}
    ${CMAKE_CURRENT_BINARY_DIR}/${executable_name})
  # Create test-specific dakota.sh for Mac to circumvent the SIP
  if(APPLE)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${target_name}.sh" 
         "#!/bin/bash\nexport DYLD_LIBRARY_PATH=$ENV{DYLD_LIBRARY_PATH}\n${CMAKE_CURRENT_BINARY_DIR}/${target_name} $@")
    execute_process(COMMAND chmod +x ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.sh)
  endif()
 
endmacro()


