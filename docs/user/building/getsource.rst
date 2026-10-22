.. _getsource:

""""""""""""""""""
Obtain Source Code
""""""""""""""""""

Aquire the Dakota source code via :ref:`downloading
<setupdakota-download>` and unpacking an archive or using Git.

.. note::

   Windows users will typically download the Windows ``.zip`` source
   archive, while all others will use the Unix ``.tar.gz`` source
   archive. This can help with compilation problems due to line ending
   style.

=============================
Cloning the Dakota Repository
=============================

Dakota's public Git repositories are hosted in the `Dakota GitHub
Organization <https://github.com/snl-dakota>`_, where ``dakota`` is
the core repository. It can be cloned using SSH
(``git@github.com:snl-dakota/dakota.git``) or HTTPS
(https://github.com/snl-dakota/dakota.git).

.. attention::

   SNL Dakota developers should instead use the repositories at
   gitlab-ex.sandia.gov/dakota

**Primary branches:** The default branch for development is
``devel``. The ``master`` branch is a stable, release-quality branch
that passed more extensive testing. Branches for specific Dakota
versions, e.g, ``6.17``, are also available.

**Submodules:** The Dakota repository is separated into public and
private meta-packages. The core or top-level repository uses Git
submodules to pull most components from public servers and
(optionally) select SNL-specific components from private servers to
create a complete Dakota checkout.

.. attention::

   If you lack full access, blindly checking out Dakota with all
   submodules may result in network errors and a partial clone. Select
   the procedure below that applies to you.

.. note::

   Substitute the server and branch names as needed in what follows.

Developers with Full Access
---------------------------

*Development team members with full access to all repositories (most
typical SNL team members).*

A full clone of Dakota (including submodules) can be checked out by
with the single command:

.. code-block::

   git clone --recursive git@gitlab-ex.sandia.gov:dakota/dakota
   cd dakota
   git checkout devel

When switching to a branch, it is always best to ensure a consistent
state of submodules, e.g.,:

.. code-block::

   git submodule update --init


Developers with Gitlab-ex Access
--------------------------------

*Development team members with access to repositories on
gitlab-ex.sandia.gov hosts (select SNL team members).*

.. code-block::

   git clone git@gitlab-ex.sandia.gov:dakota/dakota
   cd dakota
   git checkout devel

When switching to a branch, it is always best to ensure a consistent
state of submodules. However, you will need to turn off one
inaccessible submodule::

   git submodule init
   git submodule deinit local
   git submodule update


Public Access
-------------

Users and developers with access only to repositories on github.com
(most external developers and anonymous cloners).

.. code-block::

   git clone git@github.com:snl-dakota/dakota.git
   cd dakota
   git checkout devel

When switching to a branch, it is always best to ensure a consistent
state of submodules. However, only initialize the submodules you have
access to:

.. code-block::

   git submodule init packages/external
   git submodule init packages/pecos
   git submodule init packages/surfpack
   git submodule update

Or alternately submodule deinit the others (dakota-examples,
packages/local/DOT, packages/local/NLPQL, packages/local/NPSOL,
local).

**Note:** With any of the above approaches, subsequent submodule
operations such as ``submodule update`` can then be performed
automatically over the set of initialized modules without the need to
enumerate them.


.. attention::

   Git submodule update may not be appropriate when co-developing
   submodules with Dakota; see discussion later in this guide in
   :ref:`git-submodules`.

**Submodules and Git Protocols:** Top-level Dakota refers to its three
primary submodules using relative paths, so Git should by default
clone them from the same server using the same protocol as the
top-level Dakota clone (for top-level HTTPS clone, submodules should
pull via HTTPS). However, in some cases Git will want to switch to or
default to another protocol, e.g., SSH instead of HTTPS. For these
cases it may be necessary to edit .gitmodules to have absolute paths
to the submodules, e.g., https://github.com/snl-dakota/pecos, and then
run ``git submodule sync``. Or use a Git ``insteadOf`` directive in
your Git config for these repos.


