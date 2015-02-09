# Listing of Dakota tests expected to PASS on all platforms, with
# subset of fast-running smoke tests

# TODO: Move these into a file that is read (short-term) and
# eventually just annotate the test files themselves with improvements
# to the Perl script.

# Each has last recorded DEBUG timing on CASL fissile 4; number of
# CPUs required.  Timings are modulo file system contention.

# Using CASL naming conventions for now to make synchronization easier

# 0--5 sec each, 1 core
# 8 concurrent, 8 total cores, 2 batches
# est 10 sec wall
SET(DakotaExt_Dakota_TESTS_BASIC_1CORE
  dakota_4level                             # 2.61; 1
  dakota_coliny_p                           # 0.96; 1
  dakota_graphics1                          # 0.84; 1
  dakota_lib_mode                           # 2.17; 1
  dakota_lib_split                          # 0.63; 1
  dakota_multistart_ie                      # 0.84; 1
  dakota_nonlinear_cg                       # 0.66; 1
  dakota_rbdo_steel_column_analytic2_verify # 2.49; 1
  dakota_repartition                        # 1.17; 1
  dakota_resultsdb                          # 2.27; 1
  dakota_uq_incremental                     # 1.43; 1
  dakota_uq_ishigami_adapt_gsg              # 4.63; 1
  dakota_uq_monomials_sc                    # 4.26; 1
#    dakota_uq_rosenbrock_adapt_pce_lls       # FAILED
  dakota_uq_rosenbrock_adapt_sc             # 3.27; 1
  dakota_uq_textbook_sop_lhs                # 2.91; 1
  )

# 0--5 sec each, 3--10 cores
# 1 concurrent, 10 total cores, 5 batches
# est 15 sec wall
SET(DakotaExt_Dakota_TESTS_BASIC_10CORE
  dakota_big_dataset                        # 2.27; 10 (asynch explicit)
  dakota_mv                                 # 2.66; 5 (central diff)
  dakota_ncsu_textbook                      # 0.96; 8 (DiRECT asynch <=8)
  dakota_richardson                         # 1.87; 3 (asynch explicit)
  dakota_textbook_increment                 # 1.28; 5 (asynch explicit)
  )

# 5--30 sec each, 1 core
# 8 concurrent, 8 total cores, 1 batch
# est 30 sec wall
SET(DakotaExt_Dakota_TESTS_CONTINUOUS_1CORE
  dakota_cantilever_dream                   # 10.43; 1
  dakota_separable_ego_5D                   # 25.64; 1
  dakota_uq_gerstner_adapt_gsg              #  6.57; 1
  dakota_uq_ishigami_adapt_exp              # 11.96; 1
  dakota_uq_rosenbrock_adapt_exp            # 14.48; 1
  dakota_uq_rosenbrock_pce_all              # 11.88; 1
  dakota_uq_short_column_adapt_gsg          # 14.72; 1
  )

# 5--12 sec, 5 cores
# 3 concurrent, 15 total cores, 2 batches
# est 20 sec wall
SET(DakotaExt_Dakota_TESTS_CONTINUOUS_5CORE
  dakota_hybrid                             # 11.86; 5 (test 2 is asynch, pop 5)
  dakota_logratio_pce                       # 6.11; 5 (asynch explicit)
  dakota_textbook_num_derivs                # 5.31;  5 (central diff)
  dakota_uq_textbook_pce                    # 5.51; 5 (asynch explicit)
)

# 50--300 sec, 1 core
# 4 concurrent, 4 total cores, 1 batch
# est 300 sec wall
SET(DakotaExt_Dakota_TESTS_NIGHTLY_1CORE
  dakota_ouu_cantilever_lps                 #  58.14; 1
  dakota_ouu_tbch_lps                       #  65.77; 1
  dakota_rosenbrock_ego                     # 270.74; 1
  dakota_sbgouu_short_column_discrete       # 176.69; 1
  )

# 600--4000 sec, 1 core
# OMIT for now; not adding much value anyway
SET(DakotaExt_Dakota_TESTS_WEEKLY_1CORE
  dakota_uq_cantilever_sop_exp              # 3918.94; 1 (test 1, 3 time out)
  dakota_uq_short_column_sop_exp            #  681.12; 1
  )

# The test command has to be invoked without mpiexec, though the
# contained test will use mpiexec.
#
# Assume MPI gives max concurrency for now (not quite true due to asynch)
#
# 1--20 sec, 2--8 MPI x 1--5 asynch
# 2 concurrent, 16 total cores, 4 batches
# est 60 sec wall
SET(DakotaExt_Dakota_TESTS_PARALLEL
  pdakota_hybrid            #  2.98; mpi 3 x asynch 5 = 15 ?
  pdakota_lib_mode          #  7.39; mpi 4 x eval_conc = 10 x ppa = 2 ?
  pdakota_lib_split         #  7.66; 6
  pdakota_multistart_ie     # 18.11; mpi 8, ppe = 2
  pdakota_repartition       #  1.84; 5?: mpi 5, ppe 2, 5
  pdakota_sbuq_hierarchical #  3.71; 6: mpi 3, asynch 2
  pdakota_uq_textbook_lhs   #  1.18; 10: mpi 2, asynch 5
  pdakota_workdir           #  2.84; 4: mpi 3, mpi 2 x asynch 2
  )


set(dakota_smoke_tests 
  ${DakotaExt_Dakota_TESTS_BASIC_1CORE}
  ${DakotaExt_Dakota_TESTS_BASIC_10CORE}
  pdakota_hybrid 
  pdakota_repartition       
  pdakota_sbuq_hierarchical 
  pdakota_uq_textbook_lhs   
  pdakota_workdir           
  )

set(dakota_acceptance_tests 
  ${dakota_smoke_tests}
  ${DakotaExt_Dakota_TESTS_CONTINUOUS_1CORE}
  ${DakotaExt_Dakota_TESTS_CONTINUOUS_5CORE} 
  ${DakotaExt_Dakota_TESTS_NIGHTLY_1CORE}
  pdakota_lib_mode     
  pdakota_lib_split    
  pdakota_multistart_ie
  )

# Previous approach was too fragile:
#foreach(dak_test ${dakota_smoke_tests})
#  # Would be better to add all labels as we add tests instead of
#  # fragile check for above applied labels
#  # get_test_property() will return NOTFOUND for a missing test
#  get_test_property(${dak_test} LABELS dtl)
#  if (dtl MATCHES "DakotaTest")
#    set_property(TEST ${dak_test} PROPERTY LABELS "${dtl};SmokeTest")
#  endif()
#endforeach()