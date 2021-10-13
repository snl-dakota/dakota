# Define convenience variables Dakota_VERSION_{PAIR, TRIPLE, STRING, SRC}
macro(dakota_version_set_helper_vars)

  set(Dakota_VERSION_PAIR
    "${Dakota_VERSION_MAJOR}.${Dakota_VERSION_MINOR}")
  set(Dakota_VERSION_TRIPLE
    "${Dakota_VERSION_MAJOR}.${Dakota_VERSION_MINOR}.${Dakota_VERSION_PATCH}")

  # Dakota version to use for naming archives
  set(Dakota_VERSION_STRING
    "${Dakota_VERSION_TRIPLE}${Dakota_VERSION_APPEND_STABLE}")

  # Dakota version to use in source/docs (could use for tarball names?)
  # For now, we don't use .0 for new minor releases, use 5.4 not 5.4.0
  if (${Dakota_VERSION_PATCH} GREATER 0) 
    set(Dakota_VERSION_SRC 
      "${Dakota_VERSION_TRIPLE}${Dakota_VERSION_APPEND_STABLE}")
  else()
    set(Dakota_VERSION_SRC 
      "${Dakota_VERSION_PAIR}${Dakota_VERSION_APPEND_STABLE}")
  endif()

endmacro()


# Determine Git revision if working from a checkout
macro(dakota_version_from_git)

  # Building in a Git repo or a source package?
  set(DAKOTA_VERSION_file_path)

  find_package(Git)

  if(EXISTS ${Dakota_SOURCE_DIR}/.git AND GIT_FOUND)

    # Workarounds for Trilinos, where CMAKE_PROJECT_NAME != Dakota
    # and the Dakota/ directory may be symlinked.
    get_filename_component(abs_source_dir ${Dakota_SOURCE_DIR} REALPATH)
    
    # Get the abbreviated SHA1 of the most recent commit
    execute_process(COMMAND ${GIT_EXECUTABLE} log --pretty=format:%h -1 
      WORKING_DIRECTORY ${abs_source_dir}
      OUTPUT_VARIABLE Dakota_GIT_ABBREV_SHA1)
    # Get the date and time of the most recent commit
    execute_process(COMMAND ${GIT_EXECUTABLE} log --pretty=format:%ci -1 
      WORKING_DIRECTORY ${abs_source_dir}
      OUTPUT_VARIABLE Dakota_GIT_DATETIME)
    # Extract the date
    string(SUBSTRING "${Dakota_GIT_DATETIME}" 0 10 Dakota_GIT_DATE)
    # Build revision string
    ##string(CONFIGURE "@Dakota_GIT_ABBREV_SHA1@ (@Dakota_GIT_DATE@)" Dakota_GIT_REV)
    set(Dakota_GIT_REV "${Dakota_GIT_ABBREV_SHA1} (${Dakota_GIT_DATE})")

  else()

    set(Dakota_GIT_ABBREV_SHA1 "xxxxxxx")
    set(Dakota_GIT_REV "Unknown")

  endif()

  # Create VERSION file
  file( WRITE ${Dakota_BINARY_DIR}/generated/VERSION/VERSION
      "DakotaVersion ${Dakota_VERSION_SRC}
Built from GIT revision ${Dakota_GIT_REV}
" )
  set(DAKOTA_VERSION_file_path "${Dakota_BINARY_DIR}/generated/VERSION/")
  install(FILES ${Dakota_BINARY_DIR}/generated/VERSION/VERSION DESTINATION
    ${DAKOTA_TOPFILES_INSTALL})

  message(STATUS "Dakota release version is: ${Dakota_VERSION_SRC}")
  message(STATUS "Dakota git revision is: ${Dakota_GIT_REV}")


endmacro()

