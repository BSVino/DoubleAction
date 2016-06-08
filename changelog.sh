#!/bin/bash

set -e

# Before running, install the "github_changelog_generator" gem using "gem install github_changelog_generator" and make sure its binary is in the PATH.
# Also, put your github API key into a file called "github_api_key.txt".

github_api_key="$(< github_api_key.txt)"


github_changelog_generator \
    -u BSVino \
    -p DoubleAction \
    --since-tag v1.2 \
    -t "$github_api_key" \
    --no-pr-wo-labels --no-issues-wo-labels \
    --include-labels 'Server issue,Client issue' \
    --exclude-labels 'duplicate,question,invalid,wontfix,Hide from changelog,PR attached' \
    "$@"
