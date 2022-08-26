.. _git-preliminaries:

"""""""""""""""""""""""""""""
Preliminaries: Setting up Git
"""""""""""""""""""""""""""""

=======================
Basic Git configuration
=======================

By default, git does not know important information such as your name, e-mail, etc.  These can be configured globally for all git projects using the following steps:

.. code-block::

   git config --global user.name "First Last"
   git config --global user.email userid@sandia.gov


Git will use your default text editor when making commits.  You can make this explicit or change to a non-default setting using the following:

.. code-block::

   git config --global core.editor emacs


Finally, do the following to see what all of your settings are:

.. code-block::

   git config --list


=========================
Recommended push behavior
=========================

Only push current branch: The default behavior for git push differs between Git 1.x and 2.x:

- 1.x defaults to “matching” mode which pushes all local branches that match remote branches
- 2.x defaults to “simple” mode which pushes only the current branch

To avoid accidentally pushing to all remote branches, e.g., if you have local commits on both devel and a feature branch, consider setting the default behavior to simple:

.. code-block::

   # For Git 1.7 and newer
      git config --global push.default simple
   # For older Git, this behaves similarly
      git config --global push.default current


This will only push the current branch, making push more symmetric to pull.  For more info search for push.default in git help config.

====================
Pull/rebase behavior
====================

We used to recommend the following, but it causes confusion and problems when working among branches. Consider the following if you understand the implications and can make exceptions when needed. Otherwise, consider manually using git pull --rebase vs. --merge depending on the state of your local commits. It's encourage to rebase unpublished local commits on top of inbound commits from upstream when possible to keep history linear, but inevitably merge is required in some cases.

*When bringing in new changes from remote repositories, e.g., using 'git pull', git defaults to a --merge behavior.  For Dakota, it is recommended to use a --rebase mode wherein local commits are applied on top of any changes acquired from the remote repository.  This is done using the command "git pull --rebase" and the --rebase option can be made the default by setting a global config option, e.g.:*

.. code-block::

   git config --global --bool pull.rebase true

However, this affects global git behavior on any other project besides Dakota and so should only be done if this behavior is truly desireable globally. Otherwise set it in your per-repo git configuration.