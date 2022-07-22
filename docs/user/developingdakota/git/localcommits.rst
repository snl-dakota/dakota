.. _git-localcommits:

"""""""""""""""""""""""""""""""""""""
Revising local commits before pushing
"""""""""""""""""""""""""""""""""""""

One nice feature of git is that it allows you to do many things locally before pushing (publishing) to the remote repository.  One useful activity is to cleanup or otherwise better organize your local commits.  Note that you should never rebase/amend commits that have been shared with others (pushed to origin or another developer's local repo).  For more information: https://www.atlassian.com/git/tutorials/rewriting-history

=========================
Amend last commit message
=========================

You realize you could improve your commit message.  Simply do:

.. code-block::

   git commit --amend

Your text editor (vim for me) should launch and present you with the latest commit on your present branch.  At this point you are free to reword however you want.

=================
Squashing commits
=================

You have 4 local commits  (to your devel branch) that you want to "squash" into one (or more) nice, clean commit(s) before pushing to the remote repository.  For example:

- You might have checkpointed your work making a trivial comment
- You want to revise one or more commit messages
- Your 4th commit fixed a bug in the 1st commit and you want to reorder and squash: [C1, C4]; C2; C3.
- You only really made two increments of value and want to compress to two commits: [C1, C2, C3]; C4

Do:

.. code-block::

   git rebase -i HEAD~4

The "-i" option is for interactive rebasing, and HEAD~4 indicates use of the four most recent commits from the current HEAD state back.  This command will launch your text editor with the most recent four commits listed from oldest (top) to newest (bottom) along with a menu of actions to take for each commit and "pick" as the default action or each.  To preserve the commit messages from a commit, use "squash" by changing "pick" to "s" in front of each commit below the first one.  You have to have at least one "pick" present.  You can play around with the other options.

Rebasing will modify the commits, so they will get new SHA1s. Take care to not reintroduce a previously published (shared with others) commit with a new commit SHA1 in the rebase.

If you are skittish about messing up your local commits, you can play around with interactive rebasing by first creating a test branch, eg:

.. code-block::

   git checkout -b test_rebase

If the rebase fails you can exit the rebase state by doing: ``git rebase --abort``
