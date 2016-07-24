#!/bin/bash

set -e

version="$1"
if [ -z "$version" ]; then
    echo >&2 'No version given'
    exit 1
fi

tag=v"$version"

git checkout develop
git pull -r origin develop
./changelog.sh --future-release "$tag"
git commit -m "Release $version" CHANGELOG.md
git tag "$tag"
git branch -f master

git push origin "$tag" develop master
