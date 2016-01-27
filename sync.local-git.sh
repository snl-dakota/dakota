#!/bin/sh
# Clone SRN repos
git clone bayside:~rhoope/Dakota_git_srn/local local
cd local/packages/external-seat
git clone bayside:~rhoope/Dakota_git_srn/DOT DOT
cd -
cd local/packages/external-site
git clone bayside:~rhoope/Dakota_git_srn/NLPQL NLPQL
git clone bayside:~rhoope/Dakota_git_srn/NPSOL NPSOL
cd -
