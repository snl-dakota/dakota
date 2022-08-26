.. _git-gettingsourcecode:

""""""""""""""""""""""""""
Getting Dakota Source Code
""""""""""""""""""""""""""

=============================
Cloning the Dakota Repository
=============================

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

=========================
Master Branch Integration
=========================

Updating the "master" branch with changes to the "devel" branch: To preseve the stable quality of the "master" branch, a nightly Jenkins job performs a more extensive set of builds and testing on the "devel" branch followed by automated updating of "master" after successful completion.
