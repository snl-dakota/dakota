.. _git-gettingsourcecode:

""""""""""""""""""""""""""
Getting Dakota Source Code
""""""""""""""""""""""""""

See :ref:`getsource` for Git information...

.. note::

   Default branch: After cloning Dakota, git will automatically check
   out the "devel" branch. Most development changes should
   be committed to this devel branch or an appropriate topic branch

=========================
Master Branch Integration
=========================

Updating the "master" branch with changes to the "devel" branch: To
preserve the stable quality of the "master" branch, a nightly Jenkins
job performs a more extensive set of builds and testing on the "devel"
branch followed by automated updating of "master" after successful
completion.
