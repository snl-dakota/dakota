.. _setupgit-main:

"""""""""""""""""""""
Setup Git Environment
"""""""""""""""""""""

==========================
What To Do in an Emergency
==========================

If you get your Dakota project in a tangled mess and are afraid to lose any work you may have done, do something like the following for any repository you have work to salvage (eg core Dakota, pecos, local):

- Checkout a new local emergency branch, e.g. ``git checkout -b imstuck``
- Add any new files unknown to git, e.g. ``git add new_file1.hpp new_file1.cpp new_file_readme``
- Commit everything, e.g. ``git commit -am "Capturing WIP and getting help"``
- Walk away slowly
- Contact the Dakota infrastructure team, or another Git guru.

=============================
Preliminaries: Setting up Git
=============================

-----------------------
Basic Git configuration
-----------------------

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


-------------------------
Recommended push behavior
-------------------------

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

--------------------
Pull/rebase behavior
--------------------

We used to recommend the following, but it causes confusion and problems when working among branches. Consider the following if you understand the implications and can make exceptions when needed. Otherwise, consider manually using git pull --rebase vs. --merge depending on the state of your local commits. It's encourage to rebase unpublished local commits on top of inbound commits from upstream when possible to keep history linear, but inevitably merge is required in some cases.

*When bringing in new changes from remote repositories, e.g., using 'git pull', git defaults to a --merge behavior.  For Dakota, it is recommended to use a --rebase mode wherein local commits are applied on top of any changes acquired from the remote repository.  This is done using the command "git pull --rebase" and the --rebase option can be made the default by setting a global config option, e.g.:*

.. code-block::

   git config --global --bool pull.rebase true

However, this affects global git behavior on any other project besides Dakota and so should only be done if this behavior is truly desireable globally. Otherwise set it in your per-repo git configuration.

==========================
Getting Dakota Source Code
==========================

-----------------------------
Cloning the Dakota Repository
-----------------------------

The Dakota repository is separated into public and private meta-packages. The core or top-level repository uses git submodules to pull most components from public servers and select SNL-specific components from private servers to create a complete Dakota checkout.

**Note on default branch:** After cloning Dakota, git will automatically check out the "master" branch. This branch is intended to be a stable, release-quality branch that passed more extensive testing. It is not possible to commit directly to this branch. Most changes should instead be committed to the "devel" branch (or a topic branch). You can subsitute a topic branch for devel in what follows as appropriate.


Developers with Full Access
---------------------------

*Development team members with full access to all repositories (most typical SNL team members).*

A full clone of Dakota (including submodules) can be checked out by with the single command:

.. code-block::

   git clone --recursive software.sandia.gov:/git/dakota
   cd dakota
   git checkout devel

When switching to a branch, it is always best to ensure a consistent state of submodules, e.g.,:

.. code-block::

   git submodule update --init


Git submodule update may not be appropriate when co-developing submodules with Dakota; see discussion later in this guide.

Developers with Gitlab-ex Access
--------------------------------

Development team members with access to repositories on software.sandia.gov and gitlab-ex.sandia.gov hosts (select SNL team members).

After cloning you will need to turn off one inaccessible submodule:

.. code-block::

   git clone software.sandia.gov:/git/dakota 
   cd dakota
   git checkout devel


When switching to a branch, it is always best to ensure a consistent state of submodules, e.g.,:

.. code-block::

   git submodule init
   git submodule deinit local
   git submodule update


Git submodule update may not be appropriate when co-developing submodules with Dakota; see discussion later in this guide.
Developers without Gitlab-ex Access

Development team members with access only to repositories on software.sandia.gov (most external developers and anonymous cloners).

.. code-block::

   git clone software.sandia.gov:/git/dakota
   cd dakota
   git checkout devel

When switching to a branch, it is always best to ensure a consistent state of submodules. However, only initialize the submodules you have access to:

.. code-block::

   git submodule init packages/external
   git submodule init packages/pecos
   git submodule init packages/surfpack
   git submodule update

Or alternately submodule deinit the others (dakota-examples, packages/local/DOT, packages/local/NLPQL, packages/local/NPSOL, local). Git submodule update may not be appropriate when co-developing submodules with Dakota; see discussion later in this guide.

**Note:** With any of the above approaches, subsequent submodule operations such as 'submodule update' can then be performed automatically over the set of initialized modules without the need to enumerate them.

Read-only Anonymous Access
--------------------------

To access the Dakota source code without authentication, instead use HTTPS:

.. code-block::

   git clone https://software.sandia.gov/git/dakota


**Note:** Top-level Dakota refers to the three software.sandia.gov submodules using relative paths, so Git should by default clone them using the same protocol as the top-level Dakota clone (for top-level HTTPS clone, submodules should pull via HTTPS). However, in some cases Git will want to switch to or default to another protocol, e.g., SSH instead of HTTPS. For these cases it may be necessary to edit .gitmodules to have absolute paths to the submodules, e.g., https://software.sandia.gov/git/pecos, and then run git submodule sync. Or use a Git insteadOf directive in your Git config for these repos.

-------------------------
Master Branch Integration
-------------------------

Updating the "master" branch with changes to the "devel" branch: To preseve the stable quality of the "master" branch, a nightly Jenkins job performs a more extensive set of builds and testing on the "devel" branch followed by automated updating of "master" after successful completion.
