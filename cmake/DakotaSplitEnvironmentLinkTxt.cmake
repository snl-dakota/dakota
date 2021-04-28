# Split the long rpath in build/src/CMakeFiles/

file(READ ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/environment.dir/link.txt
  link_txt)

string(REGEX MATCH "packages/external/DDACE/src -Wl,-rpath,/"
  already_fixed ${link_txt})

if(NOT already_fixed)

  #string(REPLACE <match-string> <replace-string> <out-var> <input>...)
  string(REPLACE "packages/external/DDACE/src:/" "packages/external/DDACE/src -Wl,-rpath,/" link_new ${link_txt})

  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/environment.dir/link.txt
    ${link_new})

endif()
