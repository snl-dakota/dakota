.. _git-submodules:

"""""""""""""""""""
Updating Submodules
"""""""""""""""""""

When you are developing one of the submodules used by core/top-level Dakota, e.g., external, pecos, surfpack, packages/local/\*, local, the current workflow involves the following:

- ``cd`` into submodule directory, e.g., ``cd packages/pecos``
- ``git checkout devel`` (or other branch as appopriate)
- make code changes, e.g., several local commits
- push to the remote devel branch (this ensures the changes are publically available before updating core Dakota to use them)
- cd up to core Dakota
- ``git status`` should show new commits for the submodule
- ``git commit -a`` (this commits all modification including binding the sha1 of the submodule repos) or
- ``git add packages/<submodule>`` (without a trailing slash) to add only the submodule SHA1, followed by ``git commit``
- ``git push`` the change to the core Dakota repository
