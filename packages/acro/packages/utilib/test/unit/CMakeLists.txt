# Utilib tests depend on acro-utilib for a function definition
if(ENABLE_TESTS)

  include(AcroAddCxxtestRunner)

  set (runner_cxxtest_src
    TAny.cpp
    TBitArray.cpp
    TfSerialStream.cpp
    TOptionParser.cpp
    TOStreamTee.cpp
    TProperty.cpp
    TPropertyDict.cpp
    TPropertyDict_YamlPrinter.cpp
    TReferenceCounted.cpp
    TSerialize.cpp
    TSerialStream.cpp
    TSparseMatrix.cpp
    Tstring_ops.cpp
    Tstl_auxiliary.cpp
    TTuple.cpp
    TTypeManager.cpp
    TXMLSerialStream.cpp
    )

  set(runner_static_src
    # NONE
    )

  set(runner_include_directories
    ${Acro_SOURCE_DIR}/packages/utilib/src
    ${Acro_SOURCE_DIR}/tpl/cxxtest
    )

  set(runner_link_libraries utilib)

  # This call will add an executable called runner
  acro_add_cxxtest_runner(utilib
    "${runner_cxxtest_src}" "${runner_static_src}" 
    "${runner_include_directories}" "${runner_link_libraries}"
    )

  file(GLOB all_test_inputs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
    TOptionParser_*.txt Tstring_ops_wordwrap.txt)
  foreach(test_file ${all_test_inputs})
    configure_file(${test_file} ${CMAKE_CURRENT_BINARY_DIR}/${test_file} COPYONLY)
  endforeach()

endif()
