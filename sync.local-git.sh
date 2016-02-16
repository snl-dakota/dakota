#!/bin/sh
# Clone SRN repos
git clone development:~rhoope/Dakota_git_srn/local local
cd local/packages/external-seat
git clone development:~rhoope/Dakota_git_srn/DOT DOT
cd -
cd local/packages/external-site
git clone development:~rhoope/Dakota_git_srn/NLPQL NLPQL
git clone development:~rhoope/Dakota_git_srn/NPSOL NPSOL
cd -
