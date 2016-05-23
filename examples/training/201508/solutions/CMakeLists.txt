# CMakeLists.txt for DAKOTA tests

include(AddApplicationTest)
include(AddFileCopyCommand)
include(CheckIncludeFile)
include(CMakeParseArguments)

function(add_training_test _test_input_file aux_file_string extra_file_string
    copy_dakota)

  #message("add_training_test: input file=${_test_input_file}, CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
  string(REPLACE solutions exercises alt_path ${CMAKE_CURRENT_SOURCE_DIR})
  #message("And here is the alternate path=${alt_path}")
  #string(REPLACE ":" " " aux_files_list ${aux_file_string})
  #message("Would like to parse ${aux_file_list}")
  string(REGEX MATCHALL "[A-Za-z0-9_.-]+" aux_file_list ${aux_file_string})
  string(REGEX MATCHALL "[A-Za-z0-9_.-]+" ext_file_list ${extra_file_string})

  if(BUILD_TESTING AND DAKOTA_ENABLE_TESTS)

    # list of all destination files copied into test/ or subdirs test/dakota_*
    # so we can have a custom target depend on them
    set(copied_test_files_abs)

    # Configure or copy the dakota_test support files into the build
    # tree with dependency to source files (needed for legacy behavior,
    # but ultimately would like to eliminate)

    # configured primary scripts
    configure_file(${Dakota_SOURCE_DIR}/test/dakota_test.perl
      ${CMAKE_CURRENT_BINARY_DIR}/dakota_test.perl @ONLY)

    # copied primary scripts
    set(file "dakota_diff.perl")
    add_file_copy_command(${Dakota_SOURCE_DIR}/test/${file} 
      ${CMAKE_CURRENT_BINARY_DIR}/${file})
    list(APPEND copied_test_files_abs "${CMAKE_CURRENT_BINARY_DIR}/${file}")  

    # Create one CTest per DAKOTA input file, in a protected subdirectory

    set(test_input_file ${_test_input_file})

    get_filename_component(test_name ${test_input_file} NAME_WE)

    set(test_all_files "${test_input_file}")

    set(add_this_test TRUE)
    set(last_subtest -1)
    set(test_labels DakotaTest SerialTest TrainingTest)
    set(test_req_files)  # empty
    if (add_this_test)

      # Add either one test with all, or the 0-th test, followed by all sub-tests
      dakota_app_test(${test_name} ${last_subtest}
        FILE_DEPENDENCIES "${test_all_files}" "${test_req_files}"
        LABELS ${test_labels})

      # the test add makes a copy command without target, now track all
      # the copied files for later addition to a custom target
      foreach(file ${test_all_files} ${test_req_files})
        set(copied_file "${CMAKE_CURRENT_BINARY_DIR}/${test_name}/${file}")
        list(APPEND copied_test_files_abs "${copied_file}")
      endforeach()

      # Also copy the aux files into test/ to support direct testing with Perl
      set(test_aux_files ${test_all_files})
      list(REMOVE_ITEM test_aux_files ${test_input_file})
      foreach(file ${test_aux_files} ${test_req_files})
        add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/${file} 
          ${CMAKE_CURRENT_BINARY_DIR}/${file})
        list(APPEND copied_test_files_abs "${CMAKE_CURRENT_BINARY_DIR}/${file}")
      endforeach()

      file(COPY ${Dakota_SOURCE_DIR}/scripts/dprepro DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${test_name})
      foreach( aux_file ${aux_file_list})
        #message("Will copy file ${aux_file}")
        file(COPY ${alt_path}/${aux_file} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${test_name})
      endforeach()
      foreach( extra_file ${ext_file_list})
        file(COPY ${extra_file} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${test_name})
      endforeach()

    endif()  # add_this_test

    # Copy targets from src/ to test/ to facilitate testing.  We place
    # the rules for copying executables from src/ in the test/
    # directory, so they get rebuilt when make-ing in test/.  Previously
    # these were POST_BUILD actions on targets in src/.  CMake generator
    # expressions don't work in OUTPUTs, so use a manually specified
    # executable name with the CMake exe suffix
    set(test_exe_deps)
    if (copy_dakota)
      DakotaCopyTarget(dakota test_exe_deps)
    endif()

    # Create a single target to copy all the testing files
    add_custom_target(${test_name}_test_files ALL 
      DEPENDS ${copied_test_files_abs} ${test_exe_deps}
      COMMENT 
      "Copy DAKOTA test inputs, aux files, and executables to binary examples/ and subdirectories"
      )

    # Clean up test dir
    #file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/dakota_diffs.out)

  endif() # BUILD_TESTING AND DAKOTA_ENABLE_TESTS

endfunction()

add_subdirectory(calibration)
add_subdirectory(sens_analysis)
add_subdirectory(optimization)
add_subdirectory(uncertainty_analysis)
