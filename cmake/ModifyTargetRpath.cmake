# Add RPATHs to specified target_name, specifically its directory, as
# well as bin and lib up ${levels} levels.

function(ModifyTargetRpath target_name levels)
  set(path_offset "")
  foreach(i RANGE 1 ${levels} 1)
    string(CONCAT path_offset "${path_offset}/..")
  endforeach(i)
  
  if(APPLE)
    set(target_install_rpath
      "@executable_path;@executable_path${path_offset}/lib;@executable_path${path_offset}/bin")
    set_target_properties(${target_name}
      PROPERTIES "INSTALL_RPATH" "${target_install_rpath}")
  elseif(UNIX)
    set(target_install_rpath
      "\$ORIGIN:\$ORIGIN${path_offset}/lib:\$ORIGIN${path_offset}/bin")
    set_target_properties(${target_name}
      PROPERTIES "INSTALL_RPATH" "${target_install_rpath}")
  endif()

endfunction()

# Add RPATHs to specified target_name, specifically its directory, as
# well as bin and lib up three levels.

function(ModifyTargetRpath3up target_name)

  if(APPLE)
    set(target_install_rpath
      "@executable_path;@executable_path/../../../lib;@executable_path/../../../bin")
    set_target_properties(${target_name}
      PROPERTIES "INSTALL_RPATH" "${target_install_rpath}")
  elseif(UNIX)
    set(target_install_rpath
      "\$ORIGIN:\$ORIGIN/../../../lib:\$ORIGIN/../../../bin")
    set_target_properties(${target_name}
      PROPERTIES "INSTALL_RPATH" "${target_install_rpath}")
  endif()

endfunction()
