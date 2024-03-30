#!/bin/bash

# Pull Adding all changes\
git pull

# Adding all changes to staging
git add -A

# Path to the version and commit message files
VERSION_HEADER="src/version.h"
COMMIT_MESSAGE_FILE="commit.md"

# Check if the version file exists
if [ ! -f "$VERSION_HEADER" ]; then
    echo "Version header not found: $VERSION_HEADER"
    exit 1
fi

# Read version from the version file

version=$(grep '#define VERSION' "$VERSION_HEADER" | awk -F '"' '{print $2}')


# Check if the commit message file exists
if [ ! -f "$COMMIT_MESSAGE_FILE" ]; then
    echo "Commit message file not found: $COMMIT_MESSAGE_FILE"
    exit 1
fi

# Read commit message from the commit message file
commitMessage=$(cat "$COMMIT_MESSAGE_FILE")

# Format the commit message to include the version
# Github didn't support /n. So use multiline
formattedCommitMessage="$version
$commitMessage"

# Committing changes with the formatted commit message
git commit -m "$formattedCommitMessage"

# Tagging the commit with the version
git tag "$version"

# Pushing changes and tags
git push
git push origin "$version"

# Cleaning up the commit message file
tools/clean_file.sh "$COMMIT_MESSAGE_FILE"