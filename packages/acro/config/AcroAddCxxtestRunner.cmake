# Make an executable target called ${package_name}-runner but a binary
# called runner
function(acro_add_cxxtest_runner package_name
    runner_cxxtest_src runner_static_src
    runner_include_directories runner_link_libraries)

  # We would like the test binaries to depend on cxxtestgen, but can't
  # figure out
  #
  #  add_custom_target(my-cxxtestgen
  #    DEPENDS ${Acro_BINARY_DIR}/python/bin/cxxtestgen)
  #  get_target_property(CXXTEST my-cxxtestgen LOCATION)
  #  get_target_property(CXXTEST my-cxxtestgen OUTPUT_NAME)
  #
  # Generator expressions require CMake 2.8.4 or newer
  # $<TARGET_FILE:acro-cxxtestgen>

  # Generate all non-runner.cpp files
  foreach(cxxtest_src ${runner_cxxtest_src})
    list(APPEND fq_cxxtest_src ${CMAKE_CURRENT_BINARY_DIR}/${cxxtest_src})
    get_filename_component(cxxtest_base ${cxxtest_src} NAME_WE)
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${cxxtest_src}
      MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${cxxtest_base}.h
      DEPENDS ${Acro_BINARY_DIR}/python/bin/cxxtestgen
      #DEPENDS acro-cxxtestgen
      #COMMAND "$<TARGET_FILE:acro-cxxtestgen>"
      COMMAND ${Acro_BINARY_DIR}/python/bin/cxxtestgen
      ARGS --have-eh --have-std --part 
           -o ${CMAKE_CURRENT_BINARY_DIR}/${cxxtest_src}
           ${CMAKE_CURRENT_SOURCE_DIR}/${cxxtest_base}.h
      )
  endforeach()

  # Generate runner.cpp
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/runner.cpp
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/runner.h
    DEPENDS ${Acro_BINARY_DIR}/python/bin/cxxtestgen
    #DEPENDS acro-cxxtestgen
    #COMMAND ${CXXTEST} 
    COMMAND ${Acro_BINARY_DIR}/python/bin/cxxtestgen
    ARGS --have-eh --have-std --root --xunit-printer
         -o ${CMAKE_CURRENT_BINARY_DIR}/runner.cpp
	 --include=${CMAKE_CURRENT_SOURCE_DIR}/runner.h
    )

  include_directories(${runner_include_directories})

  # Static sources live in source tree and don't need full qualification
  add_executable(${package_name}-runner 
    runner.cpp ${fq_cxxtest_src} ${runner_static_src})
  set_target_properties(${package_name}-runner PROPERTIES OUTPUT_NAME runner)
  add_dependencies(${package_name}-runner acro-cxxtestgen)
  target_link_libraries(${package_name}-runner ${runner_link_libraries})

endfunction()
