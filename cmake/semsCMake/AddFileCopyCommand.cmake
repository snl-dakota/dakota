# Adds a custom command that copies a file from one location to another at build time.
#
# add_file_copy_command(
#   <source-file>
#   <destination-file>)
#
# Adds a custom command that will copy the file <source-file> to
# <destination-file> at build time.

function(add_file_copy_command source_file destination_file)
 
  # Add a command to be executed at build time that copies the source file to
  # the destination file.

  # The dependencies likely aren't managed correctly in the directory case
  if (IS_DIRECTORY ${source_file})
    add_custom_command(
      OUTPUT ${destination_file}
      COMMAND ${CMAKE_COMMAND} -E copy_directory 
        ${source_file} 
        ${destination_file}
      DEPENDS ${source_file}
      )
  else()
    add_custom_command(
      OUTPUT ${destination_file}
      COMMAND ${CMAKE_COMMAND} -E copy 
        ${source_file} 
        ${destination_file}
      DEPENDS ${source_file}
      )
  endif()

endfunction()
