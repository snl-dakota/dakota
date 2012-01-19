# Adds a target that copies a file from one location to another at build time.
#
# add_file_copy_target(
#   <target-name>
#   <source-file>
#   <destination-file>)
#
# Adds a target named <target-name> that will copy the file <source-file> to
# <destination-file> at build time.

function(add_file_copy_target target_name source_file destination_file)
 
  # Add a command to be executed at build time that copies the source file to
  # the destination file.
  add_custom_command(
    OUTPUT ${destination_file}
    COMMAND ${CMAKE_COMMAND} -E copy 
      ${source_file} 
      ${destination_file}
    DEPENDS ${source_file})
 
  # Add a target that executes the above command, and make sure it's part of
  # the default build.
  add_custom_target(${target_name} ALL
    DEPENDS ${destination_file})

endfunction()
