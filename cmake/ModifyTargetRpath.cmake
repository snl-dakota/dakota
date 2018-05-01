# Add RPATHs to specified target_name, specifically its directory, as
# well as bin and lib up three levels. For now, we only have one use
# case, so this doesn't generalize to other paths.
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
