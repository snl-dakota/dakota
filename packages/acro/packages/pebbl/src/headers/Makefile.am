
TOPINC=../../../../include
TOPBIN=${top_srcdir}/bin

all-local: copy-headers

clean-local: clean-headers

MAINTAINERCLEANFILES = Makefile.in

FULLPATH_SRC=${srcdir}/..

ALL_H_SRC=\
  $(FULLPATH_SRC)/sched/pebbl/ThreadObj.h\
  $(FULLPATH_SRC)/sched/pebbl/SelfAdjustThread.h\
  $(FULLPATH_SRC)/sched/pebbl/Scheduler.h\
  $(FULLPATH_SRC)/sched/pebbl/ThreadQueue.h\
  $(FULLPATH_SRC)/pbb/pebbl/loadBalPair.h\
  $(FULLPATH_SRC)/pbb/pebbl/parBranchThreads.h\
  $(FULLPATH_SRC)/pbb/pebbl/workerInHeap.h\
  $(FULLPATH_SRC)/pbb/pebbl/earlyOutputThread.h\
  $(FULLPATH_SRC)/pbb/pebbl/hubThread.h\
  $(FULLPATH_SRC)/pbb/pebbl/workerThread.h\
  $(FULLPATH_SRC)/pbb/pebbl/workerAuxThread.h\
  $(FULLPATH_SRC)/pbb/pebbl/stdPBBSynth.h\
  $(FULLPATH_SRC)/pbb/pebbl/spReceiver.h\
  $(FULLPATH_SRC)/pbb/pebbl/parLoadObject.h\
  $(FULLPATH_SRC)/pbb/pebbl/parPebblParams.h\
  $(FULLPATH_SRC)/pbb/pebbl/incumbSearchThread.h\
  $(FULLPATH_SRC)/pbb/pebbl/parPebblBase.h\
  $(FULLPATH_SRC)/pbb/pebbl/spServer.h\
  $(FULLPATH_SRC)/pbb/pebbl/loadBal.h\
  $(FULLPATH_SRC)/pbb/pebbl/parBranching.h\
  $(FULLPATH_SRC)/pbb/pebbl/incumbCast.h\
  $(FULLPATH_SRC)/pbb/pebbl/reposArrayInHeap.h\
  $(FULLPATH_SRC)/pbb/pebbl/reposThreads.h\
  $(FULLPATH_SRC)/pbb/pebbl/llChainer.h\
  $(FULLPATH_SRC)/pbb/pebbl/packedSolution.h\
  $(FULLPATH_SRC)/bb/pebbl/branching.h\
  $(FULLPATH_SRC)/bb/pebbl/loadObject.h\
  $(FULLPATH_SRC)/bb/pebbl/pebblBase.h\
  $(FULLPATH_SRC)/bb/pebbl/pebblParams.h\
  $(FULLPATH_SRC)/bb/pebbl/branchingGblParams.h\
  $(FULLPATH_SRC)/example/pebbl/DockingProblem.h\
  $(FULLPATH_SRC)/example/pebbl/Docking.h\
  $(FULLPATH_SRC)/example/pebbl/serialLipshitzian.h\
  $(FULLPATH_SRC)/example/pebbl/parallelLipshitzian.h\
  $(FULLPATH_SRC)/example/pebbl/parallelCore.h\
  $(FULLPATH_SRC)/example/pebbl/serialCore.h\
  $(FULLPATH_SRC)/example/pebbl/parKnapsack.h\
  $(FULLPATH_SRC)/example/pebbl/serialKnapsack.h\
  $(FULLPATH_SRC)/misc/pebbl/clusterTree.h\
  $(FULLPATH_SRC)/misc/pebbl/treeTopology.h\
  $(FULLPATH_SRC)/misc/pebbl/clustering.h\
  $(FULLPATH_SRC)/misc/pebbl/gRandom.h\
  $(FULLPATH_SRC)/misc/pebbl/BestIncumbentRepository.h\
  $(FULLPATH_SRC)/misc/pebbl/fundamentals.h\
  $(FULLPATH_SRC)/misc/pebbl/followerTree.h\
  $(FULLPATH_SRC)/misc/pebbl/scatterObj.h\
  $(FULLPATH_SRC)/misc/pebbl/chunkAlloc.h\
  $(FULLPATH_SRC)/misc/pebbl/memUtil.h\
  $(FULLPATH_SRC)/misc/pebbl/pebbl_dll.h\
  $(FULLPATH_SRC)/comm/pebbl/MessageID.h\
  $(FULLPATH_SRC)/comm/pebbl/packPointer.h\
  $(FULLPATH_SRC)/comm/pebbl/coTree.h\
  $(FULLPATH_SRC)/comm/pebbl/outBufferQ.h


includepebbldir=$(includedir)/pebbl
includepebbl_HEADERS = $(ALL_H_SRC)

copy-headers:
	@if test -d $(TOPINC) ; then : ; else mkdir $(TOPINC) ; fi
	@if test -d $(TOPINC)/pebbl ; then : ; else mkdir $(TOPINC)/pebbl ; fi
	@$(TOPBIN)/cp_u $(ALL_H_SRC) $(TOPINC)/pebbl

clean-headers:
	@rm -rf $(TOPINC)/pebbl

