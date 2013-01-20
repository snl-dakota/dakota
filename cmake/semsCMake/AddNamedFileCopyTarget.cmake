function(add_named_file_copy_target _source _destination)
  get_filename_component(filename ${_source} NAME)
  add_file_copy_target(
    copy_${filename}
    ${_source}
    ${_destination})
endfunction()