# Filter emacs *~ files from the passed list, in-place
macro(list_filter_emacs_backups file_list_name)
  foreach(fl_item ${${file_list_name}})
    if(${fl_item} MATCHES "~$")
      #message("Removing ${fl_item} from ${file_list_name}")
      list(REMOVE_ITEM ${file_list_name} ${fl_item})
    endif()
  endforeach()
endmacro()
