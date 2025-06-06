#!/bin/bash
# Initialize git repo and push initial commit to remote

REPO_URL="<YOUR_GITHUB_REPO_URL>" # TODO: replace with actual URL

set -e

if [ ! -d .git ]; then
    git init
    git branch -m main
fi

git add .

git commit -m "Initial MQTT client"

git remote add origin "$REPO_URL" || true

git push -u origin main
