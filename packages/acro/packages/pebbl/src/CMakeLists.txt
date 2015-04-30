include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/bb
                    ${CMAKE_CURRENT_SOURCE_DIR}/comm
                    ${CMAKE_CURRENT_SOURCE_DIR}/example
                    ${CMAKE_CURRENT_SOURCE_DIR}/misc
                    ${CMAKE_CURRENT_SOURCE_DIR}/pbb
                    ${CMAKE_CURRENT_SOURCE_DIR}/sched)
if(USE_MPI)
  include_directories(${MPI_INCLUDE_PATH})
endif()

add_library(pebbl
  bb/pebbl/branching.cpp
  bb/pebbl/loadObject.cpp
  bb/pebbl/pebblBase.cpp
  bb/pebbl/pebblParams.cpp
  comm/pebbl/MessageID.cpp
  comm/pebbl/coTree.cpp
  comm/pebbl/outBufferQ.cpp
  misc/pebbl/clusterTree.cpp
  misc/pebbl/clustering.cpp
  misc/pebbl/followerTree.cpp
  misc/pebbl/gRandom.cpp
  misc/pebbl/memUtil.cpp
  misc/pebbl/scatterObj.cpp
  misc/pebbl/treeTopology.cpp
  pbb/pebbl/earlyOutputThread.cpp
  pbb/pebbl/hubThread.cpp
  pbb/pebbl/incumbCast.cpp
  pbb/pebbl/incumbSearchThread.cpp
  pbb/pebbl/loadBal.cpp
  pbb/pebbl/loadBalPair.cpp
  pbb/pebbl/parBranchThreads.cpp
  pbb/pebbl/parBranching.cpp
  pbb/pebbl/parLoadObject.cpp
  pbb/pebbl/parPebblBase.cpp
  pbb/pebbl/parPebblParams.cpp
  pbb/pebbl/pbCheckpoint.cpp
  pbb/pebbl/pbEnumerate.cpp
  pbb/pebbl/pbHub.cpp
  pbb/pebbl/pbOutput.cpp
  pbb/pebbl/pbPack.cpp
  pbb/pebbl/pbWorker.cpp
  pbb/pebbl/reposThreads.cpp
  pbb/pebbl/spReceiver.cpp
  pbb/pebbl/spServer.cpp
  pbb/pebbl/workerAuxThread.cpp
  pbb/pebbl/workerInHeap.cpp
  pbb/pebbl/workerThread.cpp
  sched/pebbl/Scheduler.cpp
  sched/pebbl/SelfAdjustThd.cpp
  sched/pebbl/ThreadObj.cpp
  sched/pebbl/ThreadQueue.cpp
)
target_link_libraries(pebbl
    utilib
)

add_executable(knapMPS
  example/pebbl/knapMPS.cpp
  example/pebbl/serialKnapsack.cpp
)
target_link_libraries(knapMPS
  utilib
  pebbl
)

add_executable(lipshitzian
  example/pebbl/lipshitzian.cpp
)
target_link_libraries(lipshitzian
  utilib
  pebbl
)

add_executable(core_test
  example/pebbl/core_test.cpp
)
target_link_libraries(core_test
  utilib
  pebbl
)

add_executable(knapsack
  example/pebbl/knapsack.cpp
  example/pebbl/serialKnapsack.cpp
  example/pebbl/parKnapsack.cpp
)
target_link_libraries(knapsack
  utilib
  pebbl
)

add_executable(Docking
  example/pebbl/Docking.cpp
)
target_link_libraries(Docking
  utilib
  pebbl
)

add_executable(logAnalyze
  example/pebbl/logAnalyze.cpp
)
target_link_libraries(logAnalyze
  utilib
)

install(TARGETS pebbl EXPORT ${ExportTarget} DESTINATION lib)
# These are really testers, so are built, but not installed
#install(TARGETS knapMPS EXPORT ${ExportTarget} DESTINATION bin)
#install(TARGETS lipshitzian EXPORT ${ExportTarget} DESTINATION bin)
#install(TARGETS core_test EXPORT ${ExportTarget} DESTINATION bin)
#install(TARGETS knapsack EXPORT ${ExportTarget} DESTINATION bin)
#install(TARGETS Docking EXPORT ${ExportTarget} DESTINATION bin)
#install(TARGETS logAnalyze EXPORT ${ExportTarget} DESTINATION bin)

file(GLOB_RECURSE pebbl_headers *.h)
install(FILES ${pebbl_headers} DESTINATION include/pebbl)
