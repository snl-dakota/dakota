.. _git-submodules:

""""""""""""""""""""
Helpful git commands
""""""""""""""""""""

=======================
Status-related commands
=======================

Show the status of the current project (eg files changed, unknown files, etc.):

.. code-block::

   git status

Show the status of only files known to git (excludes unknown files):
   
.. code-block::

   git status -uno

Show the status of the submodules:

.. code-block::

   git submodule status

======================
Prune deleted branches
======================

To have your local git repositories reflect cleanup (deletion of topic branches) on the remote (removing deleted branches from your local), simply do:

.. code-block::

   git fetch --prune

To do this for all repositories, you can do the following:

.. code-block::

   cd $DAKOTA_ROOT
   git fetch --prune
   git submodule foreach git fetch --prune
   cd local
   git fetch --prune
   git submodule foreach git fetch --prune

===================
Bad rebase recovery
===================

The Git reflog command can be helpful in recovering from a bad rebase, though we recommend doing this in concert with someone from the infrastructure team who is highly Git saavy.  Helpful notes: http://stackoverflow.com/questions/134882/undoing-a-git-rebase
