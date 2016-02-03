#!/bin/bash
# Based on this gist: https://gist.github.com/domenic/ec8b0fc8ab45f39403dd
set -e  # exit with nonzero exit code if anything fails

LAST_COMMIT="$(git log -1 --pretty=%B)"
# clone build branch of Nunya
git clone -b build https://${GH_REF}

# make the .iso and copy it into the Nunya git repo (build branch
cd src && make
cd .. && cp src/*.iso Nunya
cd Nunya

git config user.name "Travis CI"
#git config user.email ""

git add .
git commit -m "${LAST_COMMIT}"

# push to build branch
git push "https://${GH_TOKEN}@${GH_REF}" build:build
