.. _git-rebasevsmerge:

""""""""""""""""""""""""""
Rebase vs. merge
""""""""""""""""""""""""""

*This is a stub article...*

In general when you have local commits that have not been shared, you should rebase them on top of incoming changes from origin/upsteam.  This includes if you're merging a local branch that has not been shared with others.  In general we should not see many messages that say "merging devel into devel"  Most times short-lived local topic branches can be fast-forward merged with devel and should not require a merge commit.

Otherwise if you're merging between branches that have been shared, e.g., merging devel to a shared topic branch like mlmfOpt or expDesignBayes, (or vice-versa), you will use git merge and create a merge commit.