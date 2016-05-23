# Filter emacs *~ files from the passed list, in-place
macro(list_filter_emacs_backups file_list)
  foreach(fl_item ${file_list})
    if(${fl_item} MATCHES "~$")
      message("Removing ${fl_item}")
      list(REMOVE_ITEM file_list ${fl_item})
    endif()
  endforeach()
endmacro()
