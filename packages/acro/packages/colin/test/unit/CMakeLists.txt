if(ENABLE_TESTS)

  include(AcroAddCxxtestRunner)

  set (runner_cxxtest_src
    TObjectType.cpp
    TRealDomain.cpp
    TIntDomain.cpp
    TGradient.cpp
    TConstraints.cpp
    TConstraintPenalty.cpp
    TDowncast.cpp
    TFiniteDifference_Real.cpp
    TFiniteDifference_MixedInt.cpp
    TLinearConstraints.cpp
    TNonD_Constraints.cpp
    TNonlinearConstraints.cpp
    TRelaxableMixedIntDomain.cpp
    TTestApplications.cpp
    TUnconMultiobj.cpp
    TWeightedSum.cpp
    TAmplApplication.cpp
    TCache.cpp
    TView_Pareto.cpp
    # TReformulation_NLP1_NLP0.cpp
    )

  set(runner_static_src
    # NONE
    )

  set(runner_include_directories
    ${Acro_SOURCE_DIR}/packages/utilib/src
    ${Acro_BINARY_DIR}/packages/utilib/src
    ${Acro_SOURCE_DIR}/packages/colin/src
    ${Acro_SOURCE_DIR}/tpl/cxxtest
    )    

  set(runner_link_libraries colin utilib)    # Need m dlopen?
  if(HAVE_AMPL)
    list(APPEND runner_link_libraries amplsolver)
  endif()

  # This call will add an executable called runner
  acro_add_cxxtest_runner(colin
    "${runner_cxxtest_src}" "${runner_static_src}" 
    "${runner_include_directories}" "${runner_link_libraries}"
    )
 

  # Auxilliary test files
  file(GLOB all_test_inputs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
    print_summary*.txt example*.xml)
  foreach(test_file ${all_test_inputs})
    configure_file(${test_file} ${CMAKE_CURRENT_BINARY_DIR}/${test_file} COPYONLY)
  endforeach()

endif()
