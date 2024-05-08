#!/bin/bash

# Define colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Ask for confirmation to pull changes
echo -e "${YELLOW}Do you want to pull the latest changes from the repository? 🔄 (y/n)${NC}"
read -r response
if [[ "$response" =~ ^[Yy]$ ]]; then
    git pull
    echo -e "${GREEN}Changes pulled successfully. ✔️${NC}"
else
    echo -e "${YELLOW}Pull skipped. Continuing without pulling changes. ⚠️${NC}"
fi

# Adding all changes to staging
git add -A
echo -e "${GREEN}All changes added to staging. ✔️${NC}"

# Path to the version and commit message files
COMMIT_MESSAGE_FILE="commit.md"
VERSION_HEADER="src/version.h"

# Check if the version file exists
if [ ! -f "$VERSION_HEADER" ]; then
    echo -e "${RED}Version header not found: $VERSION_HEADER ❌${NC}"
    exit 1
fi

# Read version from the version file
version=$(grep '#define VERSION' "$VERSION_HEADER" | awk -F '"' '{print $2}')

# Checking for commit message file
if [ -f "$COMMIT_MESSAGE_FILE" ]; then
    echo -e "${YELLOW}Commit message file found. Do you want to use the existing commit message? (Y/n) 📝${NC}"
    read -r useExistingMessage
    useExistingMessage=${useExistingMessage:-y} # default 'yes' if empty
    if [[ "$useExistingMessage" =~ ^[Yy]$ ]]; then
        commitMessage=$(cat "$COMMIT_MESSAGE_FILE")
        # Prepend version to the commit message with a newline for separation
        formattedCommitMessage="${version}
        ${commitMessage}"
    else
        echo -e "${YELLOW}Please enter your commit message: 📝${NC}"
        read -r commitMessage
        formattedCommitMessage="${commitMessage}"
    fi
else
    echo -e "${YELLOW}Commit message file not found. Please enter your commit message: 📝${NC}"
    read -r commitMessage
    formattedCommitMessage="${commitMessage}"
fi

# Committing changes
git commit -m "$formattedCommitMessage"
echo -e "${GREEN}Changes committed with version prepended to message: ✔️${NC}"

# Tagging process
echo -e "${YELLOW}Do you want to create a new release by publishing a tag? 🏷️ (y/n)${NC}"
read -r tagCommit
if [[ "$tagCommit" =~ ^[Yy]$ ]]; then
    tag=$version
    suffix=0
    while git rev-parse "$tag" >/dev/null 2>&1; do
        let "suffix+=1"
        tag="${version}.${suffix}"
    done
    git tag "$tag"
    echo -e "${GREEN}Tag assigned: $tag 🏷️${NC}"

    # Pushing changes and tag
    echo -e "${YELLOW}Do you want to push the changes and the new tag to the remote repository? 🚀 (y/n)${NC}"
    read -r pushChanges
    if [[ "$pushChanges" =~ ^[Yy]$ ]]; then
        git push
        git push origin "$tag"
        echo -e "${GREEN}Changes and new tag pushed successfully. ✔️${NC}"
    else
        echo -e "${RED}Push of changes and tag skipped. ❌${NC}"
    fi
else
    echo -e "${YELLOW}No new release will be created. Do you still want to push the changes? (y/n) 🚀${NC}"
    read -r pushChanges
    if [[ "$pushChanges" =~ ^[Yy]$ ]]; then
        git push
        echo -e "${GREEN}Changes pushed successfully without creating a new release. ✔️${NC}"
    else
        echo -e "${RED}Push skipped. ❌${NC}"
    fi
fi

# Cleaning up the commit message file, if used
if [ -f "$COMMIT_MESSAGE_FILE" ]; then
    tools/clean_file.sh "$COMMIT_MESSAGE_FILE"
fi
