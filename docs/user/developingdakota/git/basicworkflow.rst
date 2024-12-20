.. _git-basicworkflow:

"""""""""""""""""""""""""""""""
Basic Code Development Workflow
"""""""""""""""""""""""""""""""

Most changes to the Dakota source code should be made to the "devel" branch (or another local or remote topic branch).  Conceptually, the project and workflow will resemble that in the figure below, where "production releases" are simply stable states from the "devel" branch vetted and pushed to the "master" branch by a periodic automated process.  User pushes to the "master" branch are disallowed.  After creating a Dakota project as described in the previous section, the "devel" branch and associated submodules for that branch can be obtained via:

.. code-block::

   git checkout devel
   git submodule update --init

Or selective update of only initialized submodules if needed (see :ref:`Getting Dakota Source Code <git-gettingsourcecode>`):

.. code-block::

   git submodule update

See :ref:`this submodule guidance <git-submodules>` if you are actively developing in both Dakota's submodules and Dakota proper.

At this point, you can make code changes, do local commits and push your changes to the Dakota public server. 

**Commit log format:** Format your commit messages as discussed here: https://git-scm.com/book/ch5-2.html#Commit-Guidelines, for reasons discussed here: http://chris.beams.io/posts/git-commit. Key guidance (editors that are Git-aware will automate this for you):

- Short (about 50 char) one line title that indicates which capability, bug, or topic the commit relates to, followed by a blank line. It's crucial that this include what Dakota feature or area the work affects.
- Body with details explaining what/why (typically how the work was done is less important), wrapped at about 72 characters
- Include the DAK-xxxx, DAKSUP-yyyy, DP-zzzz or other issue number somewhere in the message as Jira will auto-link it

Notes:

- For instance a title “Debugging multi-map, code now compiles” leaves one wondering what map, what feature? What does this change? Consider instead something like “Optimizer options: fix memory access violation in multi-map”.  
- The association of your commit with a branch won’t be clear once it’s merged to another branch, so a short hand branch name might be a helpful part of your commit title or body, e.g., Surrogates Module or MLFnTrain.
- Repeatedly refresh this page for some amusing real-life examples of unhelpful commit messages.

**Branching Model** (Dakota implements this primarily by branching topics off of devel instead of develop off of master as shown.)

.. image:: img/workflow.png
   :alt: Example Git branch workflow

http://nvie.com/posts/a-successful-git-branching-model/

========================================
Pushing changes to the remote repository
========================================

*Note: The following applies primarily to "source code" changes. The next section provides guidance for making changes to Dakota's build/test system, which can be impractical to rigorously test prior to making commits.*

**Pre-push testing:** Before pushing to "devel," code must compile, and all RHEL8 Linux unit tests should pass and regression tests should PASS or DIFF according to the current gold standards (see dakota_assay.py).  (Other platforms need not be clean.) If this standard is too stringent, consider using a local or remote branch.  Typically you would want to pull from the origin repo (see below) and rebuild before running tests to prevent thrash integrating with others' changes.  Some helpful shortcuts (second option being fastest, but less complete):

1. Run all tests, then verify against expected gold standards using the assay script

.. code-block::

   cd build/test
   make dakota-diffs-clean
   ctest -j 4 -R dakota_
   # produce dakota_*diffs.out
   make dakota-diffs
   # compare to expected results
   source/local/scripts/dakota_assay.py \
   -s source/local/snl/sqa/artifacts/release_standards/trunk/snlfull-Linux-diffs.out \
   -a dakota_diffs.out

2. Run all tests, omitting those known to DIFF (all CTest tests should PASS):

.. code-block::

   cd build/test
   ctest  -j 4 -LE Diff

**"Okay tests are clean, I'm ready!"** When you are ready to push your changes (i.e., one or more local commits) to the remote repository, do the following steps:

- Ensure your local devel branch is current with respect to any changes made to the remote repository since your last update.  Do this even if you updated before testing. (NOTE: a convenient alias can be defined for this command):

.. code-block::

   git pull --rebase origin devel 
   ## git pull --rebase origin devel --recurse-submodules=on-demand
   git submodule update --init

- Resolve any conflicts you might have with changes from the previous step.
- Do the push:

.. code-block::

   git push

NOTE: You can do a trial run before actually doing the push by adding the --dry-run option to the previous command:

.. code-block::

   git push --dry-run

**"There are test failures or DIFFs; what next?"**

- **Minor DIFFs:** If the change induces DIFFS that the committer can easily review and bless through an RHEL8 build, typically resolve them, update commits as needed, and push to devel.  Even in this case, it may be helpful to create a local branch (git checkout -b bug_name_diffs) to separate that work in case resolution ends up being challenging or long-running.
- **Major DIFFs and/or need help** from build system or team review/resolution: Push the named branch to origin, e.g, git push origin bug_name_diffs.  Then developers can collaborate to resolve DIFFs, and/or an automated build job can be conducted on that branch.  The software engineering infrastructure team can help with assigning developers to review and resolve differences.
- **Once resolved:** Whether local or remote, when done resolving the issues and DIFFs, merge the branch into devel and push to origin.  Delete the remote (and local) branch when no longer needed.

==================
Build/Test Changes
==================

The previous section prescribes a straighforward method to vet source code changes prior to pushing. Changes to Dakota's build/test system (e.g. various CMakeLists.txt files, contents of local/cmake and cmake) are not always as easy to evaluate. They may, for example, require complex setup to "simulate" the Jenkins job environment on platforms that the developer does not have ready access to. For this reason, unlike source code changes, changes to build/test related files are not required to meet any specific standard prior to pushing them, and short term "churn" in the build system as changes are made and refined is expected and allowed. Our intention is that developers feel empowered to make small, rapid, experimental changes to the build/test system to quickly resolve problems as they emerge.

====================
For More Information
====================

Git-based workflows:

http://nvie.com/posts/a-successful-git-branching-model/

https://www.atlassian.com/git/tutorials/comparing-workflows/centralized-workflow
