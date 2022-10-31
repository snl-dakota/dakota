.. _git-emergency:

""""""""""""""""""""""""""
What To Do in an Emergency
""""""""""""""""""""""""""

If you get your Dakota project in a tangled mess and are afraid to lose any work you may have done, do something like the following for any repository you have work to salvage (eg core Dakota, pecos, local):

- Checkout a new local emergency branch, e.g. ``git checkout -b imstuck``
- Add any new files unknown to git, e.g. ``git add new_file1.hpp new_file1.cpp new_file_readme``
- Commit everything, e.g. ``git commit -am "Capturing WIP and getting help"``
- Walk away slowly
- Contact the Dakota infrastructure team, or another Git guru.