.. _git-bestpractices:

""""""""""""""""""""""""""
Recommended Best Practices
""""""""""""""""""""""""""

- Merge the content of the devel branch into topic branches "often enough" to avoid future pain associated with merging topic branches back into devel.

- To reduce the chances of merge conflicts on long-running topic branches and leading up to a merge of topic branch changes back into the devel branch (eg before a release), it is a good idea to frequently merge the devel branch into topic branches (including any submodule topic branches).  This can be attempted as easily as:

  .. code-block::

     git fetch 
     git merge origin/devel

  If the merge happens cleanly, it is then advisable  to compile the code and run the regression tests to ensure there are no regressions (NB: test baselines may have been updated by the merge).  If the merge attempt induces conflicts, you can either resolve them now or simply note the expected effort that will be required at a future time and abort the attempt via:

  .. code-block::
  
     git merge --abort