#!/bin/bash
# Based on this gist: https://gist.github.com/domenic/ec8b0fc8ab45f39403dd
set -e  # exit with nonzero exit code if anything fails

# delete .iso in out directory
# make directory if it doesn't exist
mkdir -p out
# remove everything in directory
rm -f out/*

# make the .iso
cd src && make
cd .. && cp src/*.iso out

# create new Git repo in out/
cd out
git init
git config user.name "Travis CI"
#git config user.email ""

# Only commit to the new Git repo
git add .
git commit -m "Deploy .iso"

# Force push from the current repo's master branch to the remote repo's build branch.
# (All previous history will be lost since we are overwriting it)
git push --force --quiet "https://${GH_TOKEN}@${GH_REF}" master:build > /dev/null 2>&1
